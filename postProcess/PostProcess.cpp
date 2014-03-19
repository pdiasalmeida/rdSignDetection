#include "PostProcess.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

PostProcess::PostProcess( cv::Mat filename )
{
    image=filename;
}

PostProcess::PostProcess()
{
	image= cv::Mat();
}

PostProcess::~PostProcess(){ }

//NAME OF FUNCTION: DisplayImage
//PURPOSE:
//    The function will create a window, giving it a name, 
//    and will dislay the image in this window.
//INPUT PARAMETERS:
//     name         type     value/reference               description
//--------------------------------------------------------------------------------
//     name         char*        value               the name of the window
//    image          Mat      value          the image that we want to display 
//
//OUTPUT PARAMETERS:
//     name         type     value/reference               description
//--------------------------------------------------------------------------------
//                             NO OUTPUT
//
void PostProcess::displayImage( const std::string &name, cv::Mat img )
{
    cv::namedWindow( name, 0 );
    imshow( name, img );
}

//NAME OF FUNCTION: FilterImage
//PURPOSE:
//    The function will eliminate the noise in the image, using morphological 
//    operations and the median filer. First the source image is dilated,
//    after the contours are filled, the resulted image is eroded and at the 
//    end the image is smooted by the median filter. 
//INPUT PARAMETERS:
//     name         type          value/reference               description
//--------------------------------------------------------------------------------
//                             NO INPUT
//
//OUTPUT PARAMETERS:
//     name         type          value/reference               description
//--------------------------------------------------------------------------------
//      dst          Mat             value                  the filtered image

cv::Mat PostProcess::filterImage()
{
    cv::Mat sd = getStructuringElement( cv::MORPH_CROSS, cv::Size( 4, 4 ) ); //structuring element used for dilate and erode
    dilate( image, image, sd );

    cv::Mat dst = image.clone();                                //destination image; copy of the original image
    threshold( image, image, 254, 255, CV_THRESH_BINARY );      // make the image binary

    // find the contours of the objects
    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;
    cv::findContours( image, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

    //draw the filled contours to the destination image
    cv::Scalar color( 255, 255, 255 );
    drawContours( dst, contours, -1, color, CV_FILLED, 8 );

    //Morphological operations
    //dilate
    erode( dst, dst, sd );

    //eliminate nois using a median filter
    for( int i = 0; i < 5; i++ )
        medianBlur( dst, dst, 5 );

    //output
    return dst;
}

//NAME OF FUNCTION: Elimination
//PURPOSE:
//    The function will eliminate the small objects in the image.
//    First the area of the image is computed. Secondly, for each contour
//    its area is computed and also the the aspect ratio (ration of the 
//    width and the height) of an object. If an object has area less than 
//    1/1500 of image size, and the aspect ratio outside the range of 0.25 and 1.3 then
//    we consider the region as insignificant object, therefore it is eliminated.
//INPUT PARAMETERS:
//     name         type                 value/reference               description
//---------------------------------------------------------------------------------------
//    fimage         Mat                     value          the filtered image, the output 
//                                                           image of the previous method
//   copyCont  vector<vector<Point> >      reference         the contours of the objects
//
//OUTPUT PARAMETERS:
//     name         type          value/reference               description
//--------------------------------------------------------------------------------
//     dest         Mat          value                     the image obtained after 
//                                                      elimination of the small objects
//
cv::Mat PostProcess::elimination( cv::Mat fimage, std::vector< std::vector< cv::Point > > &copyCont, long areaRatio,
		double lowAspectRatio, double highAspectRatio)
{
    long int area=image.size().area();                   //area of the original image

    cv::Mat dst = cv::Mat::zeros( fimage.rows, fimage.cols, CV_8U );   //destination image

    threshold(fimage,fimage,254,255,CV_THRESH_BINARY);   //make image binary

    //find the new contours
    std::vector< std::vector< cv::Point > > contours;
    std::vector< cv::Vec4i > hierarchy;
    findContours( fimage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE );
    
    //for each contour   
    for( unsigned int i = 0; i < contours.size(); i++ )
    {
        //associate a rectangle to calculate the width and height
        cv::Rect b = boundingRect( cv::Mat(contours[i]) );

        // compute the aspect ratio
        double ratio=(double)b.width/(double)b.height;

        long int areaRegion=b.area();

        //conditions for eliminating the objects
        if( (areaRegion<area/areaRatio) || ( (ratio>highAspectRatio) || (ratio<lowAspectRatio) ) )
        {
        	// nop
        }
        else
            copyCont.push_back( contours[i] );
    }

    //std::cout << copyCont.size() << std::endl;

    cv::Scalar color(255,255,255);
    drawContours( dst, copyCont, -1, color, CV_FILLED, 8 );

    //return image after elimination
    return dst;
}

//NAME OF FUNCTION: Convex
//PURPOSE:
//    The function will recover the shape of the segmented figures .
//    The contours of the shape is found and then the function 'convexHull2'
//    is used. 
//INPUT PARAMETERS:
//     name         type          value/reference              description
//------------------------------------------------------------------------------------
//   eimage         Mat               value            the image after the elimination   
//                                                         of the small shapes 
//   hull     vector<vector<Point>   reference        the memory storage, for each contour
//                                                   the points of  the convex hull will
//                                                             be memorated
// copyCont   vector<vector<Point>   reference          the contours of the objects
//
//
//OUTPUT PARAMETERS:
//     name         type          value/reference               description
//-------------------------------------------------------------------------------------
//     dest         Mat                value           the image obtained after elimination of 
//                                                            the recovery of the shape
//

cv::Mat PostProcess::convex( cv::Mat eimage, std::vector< std::vector< cv::Point> >& hull,
		std::vector< std::vector< cv::Point > > &copyCont )
{
    cv::Mat dest = cv::Mat::zeros( image.size(), CV_8U ); // the output image
    for( unsigned int i = 0; i < copyCont.size(); i++ )
    {
        convexHull( cv::Mat(copyCont[i]), hull[i], false );    //convex hull operation
    }

    for( unsigned int i = 0; i<copyCont.size(); i++ )   //drawing the points for convex hull
    {
        for( unsigned int j=0; j<hull[i].size(); j++ )
        {
            cv::Scalar color = cv::Scalar(255, 0, 0);
            //drawContours( drawing, copyCont, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
            //drawContours( dest, hull, i, color, 1, CV_AA, vector<Vec4i>(), 0, Point() );
            circle( dest, hull[i][j], 2, color, 1, 8 );
        }
    };

    return dest;
}


//NAME OF FUNCTION: ThresholdedContour
//PURPOSE:
//    The function will recover the shape of the segmented figures .
//    The contours of the shape is found and then the function 'convexHull2'
//    is used.
//INPUT PARAMETERS:
//     name         type          value/reference              description
//------------------------------------------------------------------------------------
//   hull     vector<vector<Point>  reference        the memory storage, for each contour
//                                                   the points of  the convex hull will
//                                                   be memorated
// Contour   vector<vector<Point>   reference        the contours of the objects
//
// extractedCont    vect<vect<pair<int,int>>>        the memory storage, for each contour that
//                                  reference        the points of the function will be stored
//
// dist_threshold   float           value            distance limit between contour points
//                                                   and convex hull edges
//OUTPUT PARAMETERS:
//     name         type            value/reference  description
//-------------------------------------------------------------------------------------
//     dest         Mat             value            the image obtained with the contour
//                                                   close to the edges of convex hull polygon

cv::Mat PostProcess::thresholdedContour( std::vector < std::vector< cv::Point > > &hull, std::vector< std::vector<cv::Point> > &Contour,
		std::vector< std::vector<cv::Point> > &outContour, float dist_threshold )
{
    cv::Mat dest = cv::Mat::zeros( image.size(), CV_8U );
    cv::Point hullCurrent, contourPoint, hullNext;

    //Computes the contourPoints
    outContour.resize( Contour.size() );
    for( unsigned int objectIndex = 0;objectIndex < Contour.size(); objectIndex++ )
    {
        int hullIndex = 0;

        hullCurrent = hull[objectIndex][hullIndex];
        contourPoint = Contour[objectIndex][0];

        //searches from the set of hull points which is also the first of the contour points
        while ( hullCurrent.x != contourPoint.x || hullCurrent.y != contourPoint.y )
        {
            hullIndex++;
            hullCurrent = hull[objectIndex][hullIndex];
        }

        //explores the hull contour in inverse order
        //std::cout << std::endl << hull[0].size() << std::endl;
        hullIndex = (( hullIndex - 1 ) + hull[objectIndex].size() ) % hull[objectIndex].size();
        hullNext = hull[objectIndex][hullIndex];

        //stores the points of each contour to object vector
        // std::vector< std::pair <int, int> > object;

        //for each point of the contour
        for( unsigned int i = 0;i < Contour[objectIndex].size();i++ )
        {
            //explore the contour point
            contourPoint = Contour[objectIndex][i];

            //if the contour point is near to the conex_hull edge add it to the output
            if ( dist_threshold >= distance( hullCurrent, hullNext, contourPoint ) )
            {
                //object.push_back( std::make_pair( contourPoint.x, contourPoint.y ) );
                outContour[objectIndex].push_back( contourPoint );
                //cout<<"distance= "<<distance(hullCurrent,hullNext,contourPoint)<<endl;
            }

            //if the explored point is the same than the Hullnext point, then change hullNext and hullcurrent
            if ( hullNext.x == contourPoint.x && hullNext.y == contourPoint.y )
            {
                hullCurrent = hull[objectIndex][hullIndex];
                hullIndex = (( hullIndex - 1 ) + hull[objectIndex].size() ) % hull[objectIndex].size();
                hullNext = hull[objectIndex][hullIndex];
            }
        }

        //extractedCont.push_back( object );
    }

    //display contour points given a threshold
//    for( unsigned int i = 0;i < extractedCont.size();i++ )
//    {
//        for ( unsigned int j = 0;j < extractedCont[i].size();j++ )
//        {
//            circle( dest, cv::Point( extractedCont[i][j].first, extractedCont[i][j].second ), 1, cvScalar( 255, 0, 0 ), 3 ); //blue
//        }
//    }

    return dest;
}

//compute the distance between the edge points (po and pf) , with the current point pc
float PostProcess::distance( cv::Point  po, cv::Point pf, cv::Point pc )
{
    float pox = ( float )po.x;
    float poy = ( float )po.y;
    float pfx = ( float )pf.x;
    float pfy = ( float )pf.y;
    float pcx = ( float )pc.x;
    float pcy = ( float )pc.y;

    // In this function, we will compute the altitude of the triangle form by the two points of the convex hull and the one of the contour.
    // It will allow us to remove points far of the convex conserving a degree of freedom

    // Compute the three length of each side of the triangle
    // a will be the base too
    float a = sqrt(pow(pfx - pox, 2.00) + pow(pfy - poy, 2.00));
    // Compute the two other sides
    float b = sqrt(pow(pcx - pox, 2.00) + pow(pcy - poy, 2.00));
    float c = sqrt(pow(pfx - pcx, 2.00) + pow(pfy - pcy, 2.00));

    // Compute S which is the perimeter of the triangle divided by 2
    float s = (a + b + c) / 2.00;

    // Compute the area of the triangle
    float area = sqrt( s*(s - a)*(s - b)*(s-c) );

    // Compute the altitude
    float altitude = 2 * area / a;

    return altitude;

    //return abs(( pfx - pox ) * ( pcy - poy ) - ( pfy - poy ) * ( pcx - pox ) ) / sqrt( pow(( pfx - pox ), 2 ) + pow(( pfy - poy ), 2 ) );
}
