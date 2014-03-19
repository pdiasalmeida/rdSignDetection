#include <opencv2/core/core.hpp>

class PostProcess
{
public:
    //Constructor and destructors
	PostProcess();
	PostProcess( cv::Mat );
    ~PostProcess();

    //Methods
    void displayImage( const std::string &, cv::Mat );

    cv::Mat filterImage();
    cv::Mat elimination( cv::Mat, std::vector< std::vector<cv::Point> >&, long, double, double );

    cv::Mat convex( cv::Mat, std::vector< std::vector<cv::Point> >&hull, std::vector< std::vector<cv::Point> > &copyCont );
    cv::Mat thresholdedContour( std::vector < std::vector< cv::Point > > &hull, std::vector< std::vector<cv::Point> > &Contour,
    		std::vector< std::vector<cv::Point> > &outContour, float dist_threshold );

    cv::Mat image;
private:
    float distance( cv::Point  po, cv::Point pf, cv::Point pc );
};
