#include "OpenNISensor.h"

int main(int argc, char** argv)
{	
	RGBDSensor* sensor = new OpenNISensor();
	cout << "Sensor initialized!" << endl;
	sensor->scan();
	delete sensor;
	return 0;
}