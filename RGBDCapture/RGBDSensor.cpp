#include "RGBDSensor.h"

void RGBDSensor::createRGBDFolders()
{
	// Create general save folder if it doesn't exist
	if (!isExistingDir(c_saveFolder))
	{
		_mkdir(c_saveFolder.c_str());
	}
	string folder = c_saveFolder + "/save";
	int i = 0;
	while (true){
		string dir = folder + to_string(i);
		if(isExistingDir(dir))
		{
			i++;
		}
		else
		{
			m_strRGBDFolder = dir;
			string dirDepth = dir + "/depth/";
			string dirColor = dir + "/rgb/";
			_mkdir(dir.c_str());
			_mkdir(dirDepth.c_str());
			_mkdir(dirColor.c_str());
			break;
		}
	}
}