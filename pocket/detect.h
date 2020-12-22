#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define EPS 0.00001
#define SHOW_PROC false
#define IMG_PATH "tmp.jpg"


using namespace cv;
using namespace std;


char* txt2jpg(char *txt_path, int w = 640, int h = 480);
char* action(char *file);