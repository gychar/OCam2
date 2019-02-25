#ifndef G_PARAM_H
#define G_PARAM_H

#include <multicam.h>
#include <ocam2_sdk.h>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTime>
#include "maths.h"
#include "zoomer.h"

/*================= GLOBAL PARAMETERS DECLARATION START=====================*/
// CONSTANT
extern const int g_mask_x;
extern const int g_mask_y;
extern const int g_imgsize;
extern const int g_mask_pixel;
extern const int g_nb_cycle;
extern const int g_statistics_try;
extern const int LENGTH_4K;
extern const int HEIGHT_4K;
extern const int PIXEL_4K_RAW;
extern const int PIXEL_4K;

// IMAGE BUFFER
extern short g_imageRawNormal[OCAM2_PIXELS_RAW_NORMAL];
extern short g_imageNormal[OCAM2_PIXELS_IMAGE_NORMAL];
extern short g_imageNormal8bits[OCAM2_PIXELS_IMAGE_NORMAL];
extern short g_imageNormalThresh[OCAM2_PIXELS_IMAGE_NORMAL];
extern short g_imageNormalDark[OCAM2_PIXELS_IMAGE_NORMAL];
extern short g_imageNormalFlat[OCAM2_PIXELS_IMAGE_NORMAL];
extern short **g_BigImageBuffer;
extern short **g_image_cycle;
extern short g_image_cycle_sum[OCAM2_PIXELS_IMAGE_NORMAL];
extern unsigned char **g_surfacebuffer;
extern unsigned short **g_surfacebuffer_short;
extern unsigned int g_buffersize;
extern vector<short> g_register_zoom_8bits;
extern vector<short> g_register_zoom_vector;
extern vector<short> g_imgNormal_vector;
extern vector<unsigned short> g_img4Kraw_vector;
extern vector<unsigned short> g_img4K_disp;
extern vector<unsigned short> g_img4K;
extern vector<unsigned short> g_img4K_pixel_val;

// QT CLASS
extern QImage *g_image;
extern QImage *g_image4K;
extern QImage *g_zoom_image;
extern QGraphicsScene *g_scene;
extern QGraphicsScene *g_scene2;
extern QGraphicsScene *g_scene_zoom;
extern QGraphicsScene *g_scene_4k;
extern QTime *g_qtimeObj;

// OCAM II
extern ocam2_rc g_rc;
extern ocam2_id g_id;

// GRABLINK
extern MCSTATUS g_status;
extern MCHANDLE g_hChannel;
extern char g_BoardIdentifier[24], g_SerialPortNumber[32];
extern int g_PciPosition, g_DriverIndex;
extern int g_sizeX;  // Width of the acquired image
extern int g_pitch;  // Pitch of the acquired image
extern int g_sizeY;  // Height of the acquired image
extern BOOL g_error;
extern void* g_SerialRefPtr;

// STATE
extern bool g_median_state;
extern bool g_ReadNoise_state;
extern bool g_dark_state;
extern bool g_flat_state;
extern bool g_run_state;
extern bool g_temp_state;
extern bool g_statistics_global_state;
extern bool g_addGaussianNoise_state;
extern bool g_surface_end;
extern bool g_zoom_full;
extern bool g_zoom_mean_done;
extern bool g_zoom_median_done;
extern bool g_zoom_sd_done;
extern bool g_load_correction;
extern bool g_zoom_show;
extern bool g_test4K;
extern bool g_select_done;
extern bool g_4kacqui;
extern bool g_pre_acqui;
extern bool g_disp4k_show;
extern bool g_sampling4k;
extern bool g_megapixel4k;
extern bool g_megapixel4k_mean;
extern bool g_megapixel4k_median;

// PARAMETERS
extern int g_BP;
extern int g_WP;
extern unsigned int g_number;
extern int g_display_frequency;
extern int g_threshvalue;
extern int g_afterthresh;
extern int g_num_frames;
extern int g_masknum;
extern int g_BigImageBufferIndex;
extern int *g_BigImageNum;
extern unsigned long g_CIRCULAR_BUFFER;
extern unsigned int g_number;
extern char g_ReadBuffer[1024];
extern QString g_data_path;
extern Maths g_maths;
extern float g_median, g_meanvalue, g_variance, g_SD, g_SNR;
extern unsigned int g_num;
extern vector<vector <float>> g_plot_I_D_Mean;
extern vector<vector <float>> g_plot_I_D_Var;
extern vector<vector <float>> g_plot_I_Var;
extern vector<float> g_plot_g_I_D_Mean;
extern vector<float> g_plot_g_I_D_Var;
extern vector<float> g_plot_g_I_Var;
extern int g_PTC_num;
extern vector<float> g_PTC_Signal;
extern vector<float> g_PTC_Sigma2Shot;
extern vector<float> g_PTC_Sigma2Read;
extern int g_MaskSetFrames;
extern int g_callback_mode;
extern QString g_temp_value[10];
extern int g_gain_value;
extern int g_n_t;
extern int g_t_length;
extern vector<vector <double>> g_mean_histo_register;
extern int test;
extern int g_zoom_id_register;
extern unsigned int g_zoom_x;
extern unsigned int g_zoom_y;
extern double g_zoom_mean;
extern double g_zoom_median;
extern double g_zoom_sd;


// NOISE
extern float g_Sigma2Read;

// TEST
extern vector<vector <unsigned short>> g_registers_4K;
extern vector<vector <unsigned short>> g_img_supp_os_4K;
extern vector<unsigned short> g_img_4K;
extern unsigned short g_save4kbuffer[8448*2055];

/*================= GLOBAL PARAMETERS DECLARATION END=====================*/


class g_param
{
public:
    g_param();


};

#endif // G_PARAM_H
