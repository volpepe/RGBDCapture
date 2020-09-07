#include "OpenNISensor.h"

Point selectedPoint{ 0,0 };

void printDepthValue(int event, int x, int y, int flag, void* param)
{
	if (event == EVENT_MOUSEMOVE)
	{
		selectedPoint = Point(x, y);
	}
}

OpenNISensor::OpenNISensor()
{
	m_flagInitSuccessful = m_flagShowImage = true;
	m_frameNum = m_frameIdx = 0;
	m_sensorType = 0;
	init();
}

OpenNISensor::~OpenNISensor()
{
	m_depthStream.stop();
	m_colorStream.stop();
	m_depthStream.destroy();
	m_colorStream.destroy();
	m_device.close();
	openni::OpenNI::shutdown();
}


bool OpenNISensor::init()
{
	openni::Status rc = openni::STATUS_OK;
	rc = openni::OpenNI::initialize();
	if (rc != openni::STATUS_OK)
	{
		std::cerr << "OpenNI Initial Error: " << openni::OpenNI::getExtendedError() << std::endl;
		openni::OpenNI::shutdown();
		m_flagInitSuccessful = false;
		return m_flagInitSuccessful;
	}


	const char* deviceURI = openni::ANY_DEVICE;
	rc = m_device.open(deviceURI);
	if (rc != openni::STATUS_OK)
	{
		cerr << "ERROR: Can't Open Device: " << openni::OpenNI::getExtendedError() << endl;
		openni::OpenNI::shutdown();
		m_flagInitSuccessful = false;
		return m_flagInitSuccessful;
	}


	rc = m_depthStream.create(m_device, openni::SENSOR_DEPTH);

	if (rc == openni::STATUS_OK)
	{
		rc = m_depthStream.start();

		if (rc != openni::STATUS_OK)
		{
			cerr << "ERROR: Can't start depth stream on device: " << openni::OpenNI::getExtendedError() << endl;
			m_depthStream.destroy();
			m_flagInitSuccessful = false;
			return m_flagInitSuccessful;
		}
	}
	else
	{
		cerr << "ERROR: This device does not have depth sensor" << endl;
		openni::OpenNI::shutdown();
		m_flagInitSuccessful = false;
		return m_flagInitSuccessful;
	}

	rc = m_colorStream.create(m_device, openni::SENSOR_COLOR);
	if (rc == openni::STATUS_OK)
	{
		rc = m_colorStream.start();
		if (rc != openni::STATUS_OK)
		{
			cerr << "ERROR: Can't start color stream on device: " << openni::OpenNI::getExtendedError() << endl;
			m_colorStream.destroy();
			m_flagInitSuccessful = false;
			return m_flagInitSuccessful;
		}
	}
	else
	{
		cerr << "ERROR: This device does not have color sensor" << endl;
		openni::OpenNI::shutdown();
		m_flagInitSuccessful = false;
		return m_flagInitSuccessful;
	}	


	if (!m_depthStream.isValid() || !m_colorStream.isValid())
	{
		cerr << "SimpleViewer: No valid streams. Exiting" << endl;
		m_flagInitSuccessful = false;
		openni::OpenNI::shutdown();
		return m_flagInitSuccessful;
	}

	openni::VideoMode depthVideoMode = m_depthStream.getVideoMode();
	openni::VideoMode colorVideoMode = m_colorStream.getVideoMode();
	m_depthWidth = depthVideoMode.getResolutionX();
	m_depthHeight = depthVideoMode.getResolutionY();
	m_colorWidth = colorVideoMode.getResolutionX();
	m_colorHeight = colorVideoMode.getResolutionY();
	cout << "Depth = (" << m_depthWidth << "," << m_depthHeight << ")" << endl;
	cout << "Color = (" << m_colorWidth << "," << m_colorHeight << ")" << endl;

	// Set exposure if needed
	m_colorStream.getCameraSettings()->setAutoWhiteBalanceEnabled(false);
	int exposure = m_colorStream.getCameraSettings()->getExposure();
	int delta = 100;
	m_colorStream.getCameraSettings()->setExposure(exposure + delta);
	m_flagInitSuccessful = true;

	return m_flagInitSuccessful;
}

void OpenNISensor::scan()
{
	if (!m_flagInitSuccessful){
		cout << "WARNING: initialize the device at first!" << endl;
		return;
	}

	cout << "Creating folders..." << endl;
	createRGBDFolders();
	cout << "Folders created!" << endl;

	string strDepthWindowName("Depth"), strColorWindowName("Color"), strVisualDepthName("Visual Depth");
	cv::namedWindow(strDepthWindowName, CV_WINDOW_AUTOSIZE);
	cv::namedWindow(strColorWindowName, CV_WINDOW_AUTOSIZE);
	cv::namedWindow(strVisualDepthName, CV_WINDOW_AUTOSIZE);
	
	int frameSkip{ 0 };

	setMouseCallback(strColorWindowName, printDepthValue, 0);

	while (true)
	{
		m_colorStream.readFrame(&m_colorFrame);
		if (m_colorFrame.isValid())
		{
			cv::Mat mImageRGB(m_colorHeight, m_colorWidth, CV_8UC3, (void*)m_colorFrame.getData());
			cv::Mat cImageBGR;
			cv::cvtColor(mImageRGB, cImageBGR, CV_RGB2BGR);
			if (m_sensorType == 0)
				cv::flip(cImageBGR, cImageBGR, 1);
			cv::imshow(strColorWindowName, cImageBGR);
			if (frameSkip == c_frameSkipMax)
			{
				cout << "Saving color frame on system..." << endl;
				cv::imwrite(m_strRGBDFolder + "/rgb/" + to_string(m_frameIdx) + ".png", cImageBGR);
			}
		}
		else
		{
			cerr << "ERROR: Cannot read color frame from color stream. Quitting..." << endl;
			return;
		}

		m_depthStream.readFrame(&m_depthFrame);
		if (m_depthFrame.isValid())
		{
			cv::Mat mImageDepth(m_depthHeight, m_depthWidth, CV_16UC1, (void*)m_depthFrame.getData());
			cv::Mat cScaledDepth;
			mImageDepth.convertTo(cScaledDepth, CV_16UC1, c_visualDepthScaleFactor);
			if (m_sensorType == 0)
			{
				cv::flip(cScaledDepth, cScaledDepth, 1);
				cv::flip(mImageDepth, mImageDepth, 1);
			}
			cv::imshow(strDepthWindowName, cScaledDepth);
			if (frameSkip == c_frameSkipMax)
			{
				cv::imwrite(m_strRGBDFolder + "/depth/" + to_string(m_frameIdx) + ".png", mImageDepth);
			}
			double depthVal = mImageDepth.at<DepthValueType>(selectedPoint) / 1000.;
			cv::Mat mDepthIndicator(100, 200, CV_16UC1);
			putText(mDepthIndicator,
				to_string(depthVal),
				Point(0,50),
				FONT_HERSHEY_DUPLEX,
				1.0,
				RGB(200, 120, 9),
				2);
			cv::imshow(strVisualDepthName, mDepthIndicator);
		}
		else
		{
			cerr << "ERROR: Cannot read depth frame from depth stream. Quitting..." << endl;
			return;
		}

		// update frameskip
		frameSkip = frameSkip == c_frameSkipMax ? 0 : frameSkip + 1;

		m_frameIdx++;
		if (cv::waitKey(1) == 27) // esc to quit
		{
			break;
		}
	}
}