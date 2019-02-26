#include "g_param.h"

/*================= GLOBAL PARAMETERS DECLARATION START=====================*/
// CONSTANT
const int g_mask_x = 60;
const int g_mask_y = 120;
const int g_imgsize = 240;
const int g_mask_pixel = g_mask_x*g_mask_y;
const int g_nb_cycle = 32;
const int g_statistics_try = 10;
const int LENGTH_4K = 8448;
const int HEIGHT_4K = 2055;
const int PIXEL_4K = 8192*2048;
const int PIXEL_4K_RAW = LENGTH_4K*HEIGHT_4K*2;

// IMAGE BUFFER
short g_imageRawNormal[OCAM2_PIXELS_RAW_NORMAL] = {0};
short g_imageNormal[OCAM2_PIXELS_IMAGE_NORMAL] = {0};
short g_imageNormal8bits[OCAM2_PIXELS_IMAGE_NORMAL] = {0};
short g_imageNormalThresh[OCAM2_PIXELS_IMAGE_NORMAL] = {0};
short g_imageNormalDark[OCAM2_PIXELS_IMAGE_NORMAL] = {0};
short g_imageNormalFlat[OCAM2_PIXELS_IMAGE_NORMAL] = {0};
short **g_BigImageBuffer;
short **g_image_cycle = new short*[g_nb_cycle];
short g_image_cycle_sum[OCAM2_PIXELS_IMAGE_NORMAL] = {0};
unsigned char **g_surfacebuffer;
unsigned short **g_surfacebuffer_short;
unsigned int g_buffersize;
vector<short> g_register_zoom_8bits(g_mask_pixel,0);
vector<short> g_register_zoom_vector(g_mask_pixel,0);;
vector<short> g_imgNormal_vector(OCAM2_PIXELS_IMAGE_NORMAL,0);
vector<unsigned short> g_img4Kraw_vector(PIXEL_4K_RAW/2,0);
vector<unsigned short> g_img4K_disp(OCAM2_PIXELS_IMAGE_NORMAL,0);
vector<unsigned short> g_img4K(8448*2055,0);
vector<unsigned short> g_img4K_pixel_val(8448*2055,0);


// QT CLASS
QImage *g_image;
QImage *g_image4K;
QImage *g_image_inter_4K;
QImage *g_zoom_image;
QGraphicsScene *g_scene;
QGraphicsScene *g_scene2;
QGraphicsScene *g_scene_zoom;
QGraphicsScene *g_scene_4k;
QGraphicsScene *g_scene_inter_4k;
QTime *g_qtimeObj;

// OCAM II
ocam2_rc g_rc;
ocam2_id g_id;

// GRABLINK
MCSTATUS g_status;
MCHANDLE g_hChannel;
char g_BoardIdentifier[24], g_SerialPortNumber[32];
int g_PciPosition, g_DriverIndex;
int g_sizeX;  // Width of the acquired image
int g_pitch;  // Pitch of the acquired image
int g_sizeY;  // Height of the acquired image
BOOL g_error;
void* g_SerialRefPtr;

// STATE
bool g_median_state = false;
bool g_ReadNoise_state = false;
bool g_dark_state = false;
bool g_flat_state = false;
bool g_run_state = false;
bool g_temp_state = false;
bool g_statistics_global_state = false;
bool g_addGaussianNoise_state = false;
bool g_surface_end = false;
bool g_zoom_full = false;
bool g_zoom_mean_done = false;
bool g_zoom_median_done = false;
bool g_zoom_sd_done = false;
bool g_load_correction = false;
bool g_zoom_show = false;
bool g_test4K = false;
bool g_select_done = false;
bool g_4kacqui = false;
bool g_pre_acqui = true;
bool g_disp4k_show = false;
bool g_disp_ratio_4k_show = false;
bool g_sampling4k = true;
bool g_megapixel4k = false;
bool g_megapixel4k_mean = false;
bool g_megapixel4k_median = false;

// PARAMETERS
int g_BP;
int g_WP;
unsigned int g_number;
int g_display_frequency = 1;
int g_threshvalue;
int g_afterthresh;
int g_num_frames = 1;
int g_masknum;
int g_BigImageBufferIndex;
int *g_BigImageNum;
char g_ReadBuffer[1024];
QString g_data_path = "/acqui/";
Maths g_maths;
float g_median, g_meanvalue, g_variance, g_SD, g_SNR;
unsigned long g_CIRCULAR_BUFFER = 60000;
unsigned int g_num = 1;
vector<vector <float>> g_plot_I_D_Mean(8, vector<float>(g_statistics_try,0));
vector<vector <float>> g_plot_I_D_Var(8, vector<float>(g_statistics_try,0));
vector<vector <float>> g_plot_I_Var(8, vector<float>(g_statistics_try,0));
vector<float> g_plot_g_I_D_Mean(g_statistics_try,0);
vector<float> g_plot_g_I_D_Var(g_statistics_try,0);
vector<float> g_plot_g_I_Var(g_statistics_try,0);
int g_PTC_num = 0;
vector<float> g_PTC_Signal(g_statistics_try,0);
vector<float> g_PTC_Sigma2Shot(g_statistics_try,0);;
vector<float> g_PTC_Sigma2Read(g_statistics_try,0);;
int g_MaskSetFrames = 0;
int g_callback_mode = 0;
QString g_temp_value[10] = {""};
int g_gain_value = 0;
int g_n_t = 101;
int g_t_length = 10;
vector<vector <double>> g_mean_histo_register(8, vector<double>(g_n_t,0));
int test = 0;
int g_zoom_id_register = 0;
unsigned int g_zoom_x = 0;
unsigned int g_zoom_y = 0;
double g_zoom_mean = 0.0;
double g_zoom_median = 0.0;
double g_zoom_sd = 0.0;

// NOISE
float g_Sigma2Read = 0;

// TEST
vector<vector <unsigned short>> g_registers_4K(8,vector<unsigned short>(1056*2055,0));
vector<vector <unsigned short>> g_img_supp_os_4K(8,vector<unsigned short>(1024*2048,0));;
vector<unsigned short> g_img_4K(4096*4096,0);
unsigned short g_save4kbuffer[] = {0};


/*================= GLOBAL PARAMETERS DECLARATION END=====================*/

g_param::g_param()
{

}
