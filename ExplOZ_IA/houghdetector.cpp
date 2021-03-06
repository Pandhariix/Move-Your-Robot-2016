#include "houghdetector.h"

HoughDetector::HoughDetector()
{
    this->factor = 10;
}

void HoughDetector::buildCartesianMap(std::vector<quint16> &distance, int factor)
{
    int x, y;

    this->factor = factor;

    this->map.release();
    this->map = cv::Mat::zeros(static_cast<int> (LIDAR_DETECTION_Y/this->factor), static_cast<int> (LIDAR_DETECTION_X/this->factor), CV_8UC1);

    for(int i=0 ; i < 180 ; i++)
    {
        if(distance[i] < 4000)
        {
            x = (static_cast<int> (distance[i]))*cos((PI/180)*(179-i))/this->factor + LIDAR_DETECTION_X/(2*this->factor);
            y = (static_cast<int> (distance[i]))*sin((PI/180)*(179-i))/this->factor;

            this->map.at<unsigned char>(y,x) = 255;
        }
    }
}

void HoughDetector::transform()
{
    float rho, theta;
    double x0, y0;
    cv::Point pt1, pt2;

    cv::dilate(this->map, this->map, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,6)));
    cv::Canny(this->map, this->map, 50, 200, 3);
    cv::HoughLines(this->map, this->detectedLines, 1, PI/180, 25);

    //DEBUG

    /*
    if(detectedLines.size() > 0)
        std::cout<<"plop"<<std::endl;
    else
        std::cout<<"nooope"<<std::endl;
    */
    /*
    for(std::vector<cv::Vec2f>::size_type i = 0; i != this->detectedLines.size(); i++)
    {
        rho = this->detectedLines[i][0];
        theta = this->detectedLines[i][1];
        x0 = rho*cos(theta);
        y0 = rho*sin(theta);

        pt1.x = cvRound(x0+1000*(-sin(theta)));
        pt1.y = cvRound(y0+1000*(cos(theta)));
        pt2.x = cvRound(x0-1000*(-sin(theta)));
        pt2.y = cvRound(y0-1000*(cos(theta)));

        cv::line(this->map, pt1, pt2, cv::Scalar(255,255,255), 3, 8);
    }
    std::cout<<this->detectedLines.size()<<std::endl;
    cv::imshow("debug console", this->map);
    */
}


void HoughDetector::getDetectedLines(std::vector<std::pair<float, float> > &lines, directionWall &wall)
{
    float rho, theta;
    double x0, y0;
    cv::Point pt1, pt2;

    int posXRobot = static_cast<int> (LIDAR_DETECTION_X/(2*this->factor));
    int minimumG = posXRobot;
    int minimumD = posXRobot;
    float resRhoG = 0;
    float resThetaG = 0;
    float resRhoD = 0;
    float resThetaD = 0;

    lines.clear();

    wall = NONE;

    bool isThereRight = false;
    bool isThereLeft = false;

    if(detectedLines.size() == 0)
    {
        cv::imshow("debug console", this->map);
        return;
    }

    for(size_t i=0 ; i<detectedLines.size() ; i++)
    {
        if( abs(static_cast<int> (this->detectedLines[i][0]/cos(this->detectedLines[i][1]))) < posXRobot && posXRobot - abs(static_cast<int> (this->detectedLines[i][0]/cos(this->detectedLines[i][1]))) < minimumG)
        {
            minimumG = posXRobot - abs(static_cast<int> (this->detectedLines[i][0]/cos(this->detectedLines[i][1])));
            resRhoG = this->detectedLines[i][0];
            resThetaG = this->detectedLines[i][1];

            isThereLeft = true;
            wall=LEFT;
        }

        if(abs(static_cast<int> (this->detectedLines[i][0]/cos(this->detectedLines[i][1]))) > posXRobot && abs(static_cast<int> (this->detectedLines[i][0]/cos(this->detectedLines[i][1]))) - posXRobot < minimumD)
        {
            minimumD = abs(static_cast<int> (this->detectedLines[i][0]/cos(this->detectedLines[i][1]))) - posXRobot;
            resRhoD = this->detectedLines[i][0];
            resThetaD = this->detectedLines[i][1];

            isThereRight = true;
            wall=RIGHT;
        }
    }

    if(isThereRight && isThereLeft)
        wall = BOTH;

    if(wall == BOTH)
    {
        lines.push_back(std::make_pair(resRhoG, resThetaG));
        lines.push_back(std::make_pair(resRhoD, resThetaD));
    }
    else if(wall == LEFT)
    {
        lines.push_back(std::make_pair(resRhoG, resThetaG));
    }
    else if(wall == RIGHT)
    {
        lines.push_back(std::make_pair(resRhoD, resThetaD));
    }

    //std::cout << resRhoD << " " << resRhoG << std::endl;


    for(std::vector<std::pair<float,float> >::iterator iter = lines.begin(); iter != lines.end() ; iter++)
    {
        rho = iter->first;
        theta = iter->second;
        x0 = rho*cos(theta);
        y0 = rho*sin(theta);

        pt1.x = cvRound(x0+1000*(-sin(theta)));
        pt1.y = cvRound(y0+1000*(cos(theta)));
        pt2.x = cvRound(x0-1000*(-sin(theta)));
        pt2.y = cvRound(y0-1000*(cos(theta)));

        cv::line(this->map, pt1, pt2, cv::Scalar(255,255,255), 3, 8);
    }
    //std::cout<<this->detectedLines.size()<<" "<<lines.size()<<std::endl;
    cv::imshow("debug console", this->map);
}
