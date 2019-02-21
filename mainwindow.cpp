/*************************************************
Author: Gaoyang CAI

Company: Laboratoire d'astrophysique de Marseille

Mail: gaoyang.cai@lam.fr

Date: 2018-09-27

Description: EMCCD development software
**************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

/*================= COMMUNICATION WITH OCAM II START =====================*/

#define BOOL int
#define TRUE 1
#define FALSE 0
#define TERMINAL_BUFFER_SIZE 1024
#define TERMINAL_STRING_SIZE 128

int g_surfacebuffercount = 0;
bool threadloop = true;

/* Read Data Thread
 * This thread will read data (if available) from the
 * Camera Link Serial port and display it in the console. */
void *MainWindow::ReadData(void * g_SerialRefPtr)
{
    int status;
    unsigned long size;
    for(int i = 0; i < 2; i++)
    {
        // Read Data
        size = TERMINAL_BUFFER_SIZE-1;
        status = clSerialRead (g_SerialRefPtr, g_ReadBuffer, &size, 1000);
        if (status != CL_ERR_NO_ERR)
        {
            printf ("clSerialRead error %d\n", status);
        }
        char *current_pos = strchr(g_ReadBuffer , '\r');
        while (current_pos)
        {
            *current_pos = '\n';
            current_pos = strchr(current_pos, '\r');
        }
        if (size>0)
        {
            g_ReadBuffer[size] = 0; 		// final char if the buffer is full
        }
        usleep (25000);
    }
}

unsigned int BaudRate2Id (unsigned int BaudRate)
{
    switch(BaudRate)
    {
    case 9600:
        return CL_BAUDRATE_9600;
    case 19200:
        return CL_BAUDRATE_19200;
    case 38400:
        return CL_BAUDRATE_38400;
    case 57600:
        return CL_BAUDRATE_57600;
    case 115200:
        return CL_BAUDRATE_115200;
    case 230400:
        return CL_BAUDRATE_230400;
    case 460800:
        return CL_BAUDRATE_460800;
    case 921600:
        return CL_BAUDRATE_921600;
    default:
        return 0;
    }
}
/*================= COMMUNICATION WITH OCAM II END=====================*/

/*================= INITIALIZATIONS START====================================*/

// Constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Mode_win = new Select_Mode;
    Mode_win->show();
    connect(Mode_win,SIGNAL(Selected()),this,SLOT(Mode_selected()));
    connect(this, SIGNAL(Acqui4K_done()),this,SLOT(Process4K()));
    ui->Snap_shot_PB->setFocus();
    qApp->installEventFilter(this);
    ui->tabWidget->setCurrentIndex(0);
    g_image = new QImage();
    g_image4K = new QImage();
    g_zoom_image = new QImage();
    g_scene = new QGraphicsScene;
    g_scene2 = new QGraphicsScene;
    g_scene_zoom = new QGraphicsScene;
    g_scene_4k = new QGraphicsScene;
    g_qtimeObj = new QTime;

    Init_Driver();
    //    InitializeMultiCam();
    //    Ocam_Init();
    //    SerialInit();
    //    InitBigImageBuffer();
    g_BP = 0;
    g_WP = 16383;

}

// Slot function
void MainWindow::Mode_selected()
{
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(onTimeOut()));
    timer->start(1000);
    InitializeMultiCam();
    Ocam_Init();
    SerialInit();
    //    sleep(1);
    //    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    //    if(g_status != MC_OK){
    //        test_error();
    //    }
    InitBigImageBuffer();
    this->hide();
    this->show();
    cout << "ok" << endl;

}

void MainWindow::Process4K()
{
    for(int i = 0; i < PIXEL_4K_RAW/2; i++){
        int x = i % 8448;
        int y = i / 8448;
        int reg = x / 1056;
        g_registers_4K[reg][x-reg*1056+1056*y] = g_img4Kraw_vector[i];
        //        if(g_registers_4K[reg][x-reg*1056+1056*y] != g_img4Kraw_vector[i])
        //            cout << dec << i << " --- " << reg << " --- " <<
        //                    x-reg*1056+1056*y << " --- " << g_registers_4K[reg][x-reg*1056+1056*y] <<
        //                    " --- " << g_img4Kraw_vector[i] << endl;
    }
    //    for(int i = 0; i < 1056*2055; i++){
    //        cout << dec << i << " " << g_registers_4K[0][i] << endl;
    //    }
    cout << "Split ok" << endl;
    for(int i = 0; i < 8; i++){
        int k = 0;
        for(int j = 0; j < 1056*2055; j++){
            if(j > 7391 && (j % 1056) > 31 ){
                g_img_supp_os_4K[i][k] = g_registers_4K[i][j];
                //                cout << dec << j << " --- " <<g_img_supp_os_4K[i][k] << " --- " << g_registers_4K[i][j] << endl;
                k++;
            }
        }
    }
    cout << "Supp OS ok" << endl;
    for(int i = 0; i < 8; i++){
        int offset_x = i % 4;
        int offset_y = i / 4;
        for(int j = 0; j < 1056*2048; j++){
            int x = j % 1056;
            int y = j / 1056;
            g_img_4K[x + offset_x * 1056 + (y + offset_y * 2048) * 4096] = g_img_supp_os_4K[i][j];
        }
    }
    cout << "Combine ok" << endl;
}

// Destructor
MainWindow::~MainWindow()
{
    // Stop the channel
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ORPHAN);
    if(g_status != MC_OK){
        test_error();
    }
    // Delete the channel
    g_status = McDelete(g_hChannel);
    if(g_status != MC_OK){
        test_error();
    }
    // Terminate the driver
    g_status = McCloseDriver();
    if(g_status != MC_OK){
        test_error();
    }
    // Closes the serial device and cleans up the resources associated with serialRef.
    g_status = clSerialClose(g_SerialRefPtr);
    // Delete objects
    delete g_image;
    delete g_scene;
    delete g_scene2;
    delete g_qtimeObj;
    delete ui;
}

// Test
void MainWindow::on_Test_PB_clicked()
{
    cout << "start" << endl;
    bool b = false;
    bool c = b ? !b : b;
    cout << c << endl;
}

// GrabLink Driver Callback Function
void MainWindow::McCallback(PMCCALLBACKINFO CallBackInfo){
    MCHANDLE hSurface;
    //    BYTE *pImage = (BYTE *)malloc(sizeof(BYTE)*(PIXEL_4K));
    BYTE *pImage;
    switch(CallBackInfo->Signal){
    case MC_SIG_SURFACE_PROCESSING:
    {
        hSurface = static_cast<MCHANDLE>(CallBackInfo->SignalInfo);
        McGetParamPtr(hSurface,MC_SurfaceAddr,(PVOID*)&pImage);
        //        for(int i = 0; i < 2*PIXEL_4K; i++){
        //            if(i < PIXEL_4K)
        //            cout << dec << i << "    " << hex << int(*(pImage+i)) << endl;
        //        }
        //        if(g_pre_acqui){
        //            g_pre_acqui = false;
        //        }else{
        // Stock current frame
        if(g_callback_mode == 0){
            if(!g_test4K){
                // Merge two 8-bit char to a 16-bit short
                for(int j = 0; j < OCAM2_PIXELS_RAW_NORMAL; j++){
                    g_imageRawNormal[j] = (static_cast<short>(*(pImage+j*2+1)<<8)) | *(pImage+j*2);
                }
                ocam2_descramble(g_id, &g_number, g_imageNormal, g_imageRawNormal);
                // Convert to 14 bits data
                for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
                    g_surfacebuffer_short[g_surfacebuffercount][i] = g_imageNormal[i] & 0x3fff;
                }
                if(g_surfacebuffercount < g_buffersize - 1){
                    g_surfacebuffercount++;
                }else{
                    g_surface_end = true;
                    McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_IDLE);
                }
            }else{
                for(int j = 0; j < g_sizeX*g_sizeY/2; j++){
                    int reg = j / (1056 * 2055);
                    int pos = j - reg * (1056 * 2055);
                    int x = pos % 1056 + reg * 1056;
                    int y = pos / 1056;
                    int index = x + y * 8448;
                    g_surfacebuffer_short[g_surfacebuffercount][index] =  (static_cast<unsigned short>(*(pImage+j*2+1))) << 8 | *(pImage+j*2);
                }
                g_4kacqui = true;
                if(g_surfacebuffercount < g_buffersize - 1){
                    g_surfacebuffercount++;
                }else{
                    g_surface_end = true;
                    McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_IDLE);
                }
            }
        }
        // display current frame
        else if(g_callback_mode == 1){
            if(!g_test4K){
                // Merge two 8-bit char to a 16-bit short
                for(int i = 0;i<OCAM2_PIXELS_RAW_NORMAL;i++){
                    g_imageRawNormal[i] = (static_cast<short>(((*(pImage+(i*2+1))))<<8)) |((*(pImage+(i*2))));
                }
            }else{
                //                for(int j = 0; j < g_sizeX*g_sizeY/2; j++){
                //                    g_img4Kraw_vector[j] =  (static_cast<unsigned short>(*(pImage+j*2+1))) << 8 | *(pImage+j*2);
                //                }
                for(int j = 0; j < g_sizeX*g_sizeY/2; j++){
                    int reg = j / (1056 * 2055);
                    int pos = j - reg * (1056 * 2055);
                    int x = pos % 1056 + reg * 1056;
                    int y = pos / 1056;
                    int index = x + y * 8448;
                    g_img4K[index] =  (static_cast<unsigned short>(*(pImage+j*2+1))) << 8 | *(pImage+j*2);
                }
                g_4kacqui = true;
                //                qDebug() << g_qtimeObj->currentTime().toString();
            }
        }
        break;
        //        }
    }
    case MC_SIG_ACQUISITION_FAILURE:
        fprintf(stderr, "Acquisition Failure. Is a video source connected?\n");
        g_error = TRUE;
        break;
    default:
        fprintf(stderr,"Signal not handled: %d", CallBackInfo->Signal);
        g_error = TRUE;
        break;
    }
}


void *MainWindow::SignalHandler ( void * Param){
    MCSIGNALINFO sigInfo;
    while (threadloop)
    {
        // Get Signal information
        if (McWaitSignal(g_hChannel, MC_SIG_ANY,1000, &sigInfo ) == MC_OK)
            MainWindow::McCallback(&sigInfo);
    }
}



// GrabLink Driver Initialization Function
void MainWindow::Init_Driver(){
    cout << "GrabLink test program started..." << endl;
    ChangeSection();

    // Initialize MultiCam Driver
    cout << "Opening driver...";
    g_status = McOpenDriver(nullptr);
    if(g_status != MC_OK)
        test_error();
    cout << "Done" << endl;
    g_status = McGetParamInt(MC_CONFIGURATION,MC_DriverIndex,&g_DriverIndex);
    if(g_status!=MC_OK){
        test_error();
    }
    cout << "Driver Index:" << g_DriverIndex << endl;
    ChangeSection();

    // Show board informations
    g_status = McGetParamStr(MC_CONFIGURATION,MC_BoardIdentifier,g_BoardIdentifier,sizeof(g_BoardIdentifier));
    if(g_status!=MC_OK){
        test_error();
    }
    cout << "Board Identifier:" << g_BoardIdentifier << endl;
    g_status = McGetParamInt(MC_CONFIGURATION,MC_PciPosition,&g_PciPosition);
    if(g_status!=MC_OK)
        test_error();
    cout << "PCI position:" << hex << g_PciPosition << endl;
    ChangeSection();
}

// GrabLink MultiCam Initialize Function
int MainWindow::InitializeMultiCam(){
    g_sizeX = 0;
    g_sizeY = 0;
    // Create a channel
    g_status = McCreate(MC_CHANNEL,&g_hChannel);
    if(g_status != MC_OK){
        test_error();
        return -1;
    }

    // Link the channel to the board
    g_status = McSetParamInt(g_hChannel,MC_DriverIndex,0);
    if(g_status != MC_OK){
        test_error();
        return -1;
    }

    // Set the connector to 'M'
    g_status = McSetParamStr(g_hChannel,MC_Connector,"M");
    if(g_status != MC_OK){
        test_error();
        return -1;
    }

    // Choose the CAM file
    if(!g_test4K){
        g_status = McSetParamStr(g_hChannel,MC_CamFile,"/home/gcai/LAM/Qt/Ocam/MCL");
        cout << "CAM File MCL selected." << endl;
    }else{
        g_status = McSetParamStr(g_hChannel,MC_CamFile,"/home/gcai/LAM/Qt/Ocam/MCL4K");
        cout << "CAM File MCL4K selected." << endl;
    }
    if(g_status != MC_OK){
        test_error();
        return -1;
    }

    ChangeSection();

    // Retrieve channel size information.
    g_status= McGetParamInt(g_hChannel, MC_ImageSizeX, &g_sizeX);
    if(g_status != MC_OK){
        test_error();
        return -1;
    }
    g_status= McGetParamInt(g_hChannel, MC_ImageSizeY, &g_sizeY);
    if(g_status != MC_OK){
        test_error();
        return -1;
    }
    g_status= McGetParamInt(g_hChannel, MC_BufferPitch, &g_pitch);
    if(g_status != MC_OK){
        test_error();
        return -1;
    }

    // Register our Callback function for the MultiCam asynchronous signals.
    g_status = McRegisterCallback(g_hChannel, McCallback, nullptr);
    if(g_status != MC_OK){
        test_error();
        return -1;
    }

    // Enable the signals we need:
    // MC_SIG_SURFACE_PROCESSING: acquisition done and locked for processing
    // MC_SIG_ACQUISITION_FAILURE: acquisition failed.
    g_status = McSetParamInt(g_hChannel, MC_SignalEnable + MC_SIG_SURFACE_PROCESSING, MC_SignalEnable_ON);
    if(g_status != MC_OK){
        test_error();
        return -1;
    }
    g_status = McSetParamInt(g_hChannel, MC_SignalEnable + MC_SIG_ACQUISITION_FAILURE, MC_SignalEnable_ON);
    if(g_status != MC_OK){
        test_error();
        return -1;
    }
    //    g_status = McSetParamInt(g_hChannel, MC_SignalEnable + MC_SIG_SURFACE_FILLED, MC_SignalEnable_ON);
    //    if(g_status != MC_OK){
    //        test_error();
    //        return -1;
    //    }

    return 0;
}

// GrabLink Test Error Function
void MainWindow::test_error(){
    cout << "Error:" << dec << g_status << endl;
}

// OCam II Initialization Function
void MainWindow::Ocam_Init(){
    g_rc=ocam2_init(OCAM2_NORMAL, "/home/gcai/ocam-simu/srcdir/libocam2sdk/ocam2_descrambling.txt", &g_id);
}

// Initialize Serial Communication
void MainWindow::SerialInit(){
    int status = -1;
    unsigned int i, BaudRate, BaudRateId;
    unsigned long size = TERMINAL_STRING_SIZE;
    unsigned int NumPorts, PortId;
    char PortName[TERMINAL_STRING_SIZE];
    // Enumerate Camera Link Serial Ports
    status = clGetNumSerialPorts (&NumPorts);
    if (status != CL_ERR_NO_ERR)
    {
        printf ("clGetNumSerialPorts error %d\n", status);
    }
    if (NumPorts == 0)
    {
        printf ("\nSorry, no serial port detected.\n");
        printf ("Check if a GrabLink is present in your system and if the drivers are correctly loaded...\n");
    }
    printf ("\nDetected ports:\n");
    for (i=0; i<NumPorts; i++)
    {
        status = clGetSerialPortIdentifier (i, PortName, &size);
        printf (" - Serial Index %d : %s\n", i, PortName);
    }
    // Camera Link Serial Port Selection, we choose the first port.
    PortId = 0;
    printf ("Port selected : %d\n", PortId);
    // Initialize Camera Link Serial Connetion
    status = clSerialInit (PortId, &g_SerialRefPtr);
    if (status != CL_ERR_NO_ERR)
    {
        printf ("clSerialInit error %d\n", status);
    }
    // Camera Link Serial Port Baudrate Selection
    BaudRate = 115200;
    BaudRateId=BaudRate2Id(BaudRate);
    printf ("Baudrate selected : %d (BaudRateId=%d)\n", BaudRate, BaudRateId);
    // Set Camera Link Serial Port Baudrate
    status = clSetBaudRate (g_SerialRefPtr, BaudRateId);
    if (status != CL_ERR_NO_ERR)
    {
        printf ("clSetBaudRate error %d\n", status);
    }
}

// Big Image Buffer Initialization Function
void MainWindow::InitBigImageBuffer(){
    g_BigImageBufferIndex = 0;
    if(!g_test4K){
        g_BigImageBuffer = new short *[g_CIRCULAR_BUFFER];
        g_BigImageNum = new int[g_CIRCULAR_BUFFER];
        for(unsigned long i = 0; i < g_CIRCULAR_BUFFER; i++){
            g_BigImageBuffer[i] = new short[OCAM2_PIXELS_IMAGE_NORMAL];
            memset(g_BigImageBuffer[i],0,OCAM2_PIXELS_IMAGE_NORMAL*sizeof(short));
        }
    }else{
        g_CIRCULAR_BUFFER = 120;
        g_BigImageBuffer = new short *[g_CIRCULAR_BUFFER];
        g_BigImageNum = new int[g_CIRCULAR_BUFFER];
        for(unsigned long i = 0; i < g_CIRCULAR_BUFFER; i++){
            g_BigImageBuffer[i] = new short[8448*2055];
            memset(g_BigImageBuffer[i],0,8448*2055*sizeof(short));
        }
    }
}

/*================= INITIALIZATIONS END ====================================*/

/*================= FUNCTIONS START ======================================*/

// OCam II Acquire Image Function, the captured image is stored in g_imageNormal (16-bits).
// With dark operation, median filter, threshold function and pixel correction function.
// Final output is stored in g_imageNormal8bits (8-bits) for display.
void MainWindow::AcquireImages()
{
    g_callback_mode = 1;
    // Start Acquisitions for this channel.
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    if(g_status != MC_OK){
        test_error();
    }
    ocam2_descramble(g_id, &g_number, g_imageNormal, g_imageRawNormal);
    // Convert to 14 bits data
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        //        cout << i << "   " << g_imageNormal[i] << endl;
        g_imageNormal[i] = g_imageNormal[i] & 0x3fff;
        //        cout << i << "   " << g_imageNormal[i] << endl;
    }
    // Dark Function (g_imageNormal 16-bits)
    if(g_dark_state){
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
            g_imageNormal[i] = g_imageNormal[i] - g_imageNormalDark[i];
            if(g_imageNormal[i] < 0)
                g_imageNormal[i] = 0;
        }
    }
    // Flat Function
    if(g_flat_state){
        double min = 16383, max = 0;
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
            if(double(g_imageNormal[i]) / double(g_imageNormalFlat[i]) < min)
                min = double(g_imageNormal[i]) / double(g_imageNormalFlat[i]);
            if(double(g_imageNormal[i]) / double(g_imageNormalFlat[i]) > max)
                max = double(g_imageNormal[i]) / double(g_imageNormalFlat[i]);
        }
        //        cout << dec << min << " " << max << endl;
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
            //            cout << dec << i << "th pixel, " << g_imageNormal[i] << " / " << g_imageNormalFlat[i] << " = ";
            //            cout << dec << double(g_imageNormal[i]) / double(g_imageNormalFlat[i]) << "    ";
            //            cout << dec << (double(g_imageNormal[i]) / double(g_imageNormalFlat[i]) - min) / (max) * 16383 << endl;
            g_imageNormal[i] = (double(g_imageNormal[i]) / double(g_imageNormalFlat[i]) - min) / (max) * 16383;
            //            g_imageNormal[i] = (double(g_imageNormal[i]) / double(g_imageNormalFlat[i]))*1000;
        }
    }
    // Copy to vector
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        g_imgNormal_vector[i] = g_imageNormal[i];
    }
    // Add Gaussian Noise
    if(g_addGaussianNoise_state){
        float mu = 29, sigma = 0.1;
        // Random device class instance, source of 'true' randomness for initializing random seed
        random_device rd{};
        // Mersenne twister PRNG, initialized with seed from previous random device instance
        mt19937_64 gen{rd()};
        // Instance of class std::normal_distribution with specific mean and standard deviation
        normal_distribution<float> gaussian_noise(mu,sigma);
        // Add the gaussien noise to image
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            g_imageNormal[i] += gaussian_noise(gen);
        }
    }
    // Median Filter Function (g_imageNormal 16-bits)
    if(g_median_state){
        short medianval = g_maths.Median(short2float(g_imageNormal,OCAM2_PIXELS_IMAGE_NORMAL));
        //        cout << dec << "median =  " << medianval << endl;
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
            if(g_imageNormal[i]<medianval){
                g_imageNormal[i]= static_cast<short>(g_BP);
            }
        }
    }
    // Read Noise Suppression
    if(g_ReadNoise_state){
        float readnoise = 3*sqrt(g_Sigma2Read);
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
            g_imageNormal[i] -= readnoise;
            if(g_imageNormal[i] < 0)
                g_imageNormal[i] = 0;
        }
    }
    // g_imageNormalThresh 16-bits
    //    threshold_function(short(ui->Threshold_Slider->value()));
    // g_imageNormal8bits 8-bits
    pixel_correction(g_imageNormal,g_imageNormal8bits);
    ui->Thresh_Label->setText(QString::number(g_threshvalue) + " % after thresh = " + QString::number(g_afterthresh));
    g_num++;
}

void MainWindow::AcquireImages4K()
{
    g_callback_mode = 1;
    // Start Acquisitions for this channel.
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    //    for(int j = 0; j < 1056*8; j++){
    //        if(g_img4Kraw_vector[j]!=0)
    //            cout << dec << j << " " << hex << g_img4Kraw_vector[j] << dec << " " << g_img4Kraw_vector[j]<< endl;
    //    }
    test2();
}


// Display Image Function, images are 8-bits.
void MainWindow::display(const short imagebuffer[]){
    *g_image = QImage(240,240,QImage::Format_RGB888);
    for (int pos=0,i=0; i < OCAM2_PIXELS_IMAGE_NORMAL; pos+=3,i++)
    {
        unsigned char PixRed=0, PixGreen=0, PixBlue=0;
        short RawPixel= imagebuffer[i];
        if (RawPixel >= 0xFF) //if pixel is above White point
            PixRed= PixGreen= PixBlue= 0xFF;//8 bit value is 255
        else if (RawPixel <= 0)//if below Black
            PixRed= PixGreen= PixBlue = 0;//value is 0
        else{
            PixRed= PixGreen= PixBlue= static_cast<unsigned char>(static_cast<int>(RawPixel));
        }
        g_image->bits()[pos] = PixRed;
        g_image->bits()[pos+1] = PixGreen;
        g_image->bits()[pos+2] = PixBlue;
    }
    g_scene->clear();
    g_scene->addPixmap(QPixmap::fromImage(*g_image));
    ui->graphicsView->setScene(g_scene);
    ui->graphicsView->show();
}

void MainWindow::display_4K(const vector<unsigned short> imagebuffer)
{
    *g_image = QImage(240,240,QImage::Format_RGB888);
    for (int pos=0,i=0; i < OCAM2_PIXELS_IMAGE_NORMAL; pos+=3,i++)
    {
        unsigned char PixRed=0, PixGreen=0, PixBlue=0;
        short RawPixel= imagebuffer[i];
        //        short RawPixel= imagebuffer[i] - 15361;
        //        cout << dec << i << " " << RawPixel << endl;
        //        RawPixel -= 768;
        if (RawPixel >= 0xFF) //if pixel is above White point
            PixRed= PixGreen= PixBlue= 0xFF;//8 bit value is 255
        else if (RawPixel <= 0)//if below Black
            PixRed= PixGreen= PixBlue = 0;//value is 0
        else{
            PixRed= PixGreen= PixBlue= static_cast<unsigned char>(static_cast<int>(RawPixel));
        }
        g_image->bits()[pos] = PixRed;
        g_image->bits()[pos+1] = PixGreen;
        g_image->bits()[pos+2] = PixBlue;
    }
    g_scene->clear();
    g_scene->addPixmap(QPixmap::fromImage(*g_image));
    ui->graphicsView->setScene(g_scene);
    ui->graphicsView->show();

}

void MainWindow::display_4K_full(const vector<unsigned short> imagebuffer)
{
}

// Threshold Function
void MainWindow::threshold_function(short thresh){
    g_afterthresh = 0;
    if(thresh != 0){
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
            g_imageNormalThresh[i] = short(g_BP);
        }
        int thresh_value = (g_WP-g_BP)*thresh/100+g_BP;
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
            if(g_imageNormal[i]>=short(thresh_value) && g_imageNormal[i] <= short(g_WP)){
                g_imageNormalThresh[i] = g_imageNormal[i];
                g_afterthresh++;
            }else if(g_imageNormal[i] > short(g_WP)){
                g_imageNormalThresh[i] = short(g_WP);
                g_afterthresh++;
            }
        }
    }else{
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
            g_imageNormalThresh[i] = g_imageNormal[i];
        }
        g_afterthresh = OCAM2_PIXELS_IMAGE_NORMAL;
    }
}

// Adjust pixel value to 8-bits
void MainWindow::pixel_correction(short img1[], short img2[]){
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        // Inverse 2 bit
        int temp = img1[i];
        temp = ((temp-(int)g_BP)*255);
        temp = temp/((int)g_WP-(int)g_BP);
        if(temp < 0)
            img2[i] = 0;
        else if(temp > 255)
            img2[i] = 255;
        else
            img2[i] = static_cast<short>(temp);
    }
}

void MainWindow::pixel_correction_4K(vector<short> img1, vector<short> img2)
{
    cout << g_BP << " " << g_WP << endl;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        if(img1[i] > 15360){
            img2[i] = img1[i] - 15361;
        }else{
            img2[i] = img1[i];
        }
    }
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        if(img2[i] / 256 > 0){
            img2[i] = img2[i] % 256;
        }
    }
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        cout << i << " " << img2[i] << endl;
    }
    //    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
    //        // Inverse 2 bit
    //        int temp = img1[i];
    //        temp = ((temp-(int)g_BP)*255);
    //        temp = temp/((int)g_WP-(int)g_BP);
    //        if(temp < 0)
    //            img2[i] = 0;
    //        else if(temp > 255)
    //            img2[i] = 255;
    //        else
    //            img2[i] = static_cast<short>(temp);
    //    }
}

// Transfer Serial Command Function
void MainWindow::SerialCommand(QString command, int mode){
    // Using c++ 11 thread
    int status;
    unsigned long size;
    size = 1;
    int length = command.size();
    char c;
    QByteArray strba = command.toLatin1();
    memset (g_ReadBuffer, 0, TERMINAL_BUFFER_SIZE*sizeof(char));
    for(int i = 0; i < length; i++){
        c = strba.at(i);
        status = clSerialWrite(g_SerialRefPtr,&c,&size,1000);
    }
    c = '\r';
    status = clSerialWrite(g_SerialRefPtr,&c,&size,1000);
    std::thread ReadDataThreadId(&MainWindow::ReadData,g_SerialRefPtr);
    ReadDataThreadId.join();
    if(mode == 0){
        // Check interface mode
        if(g_ReadBuffer[0] != '[' && g_ReadBuffer[0] != '<' && g_ReadBuffer[0] != '\0'){
            memset(strstr(g_ReadBuffer,"\n\nOk :"), '\0', sizeof(strstr(g_ReadBuffer,"\n\n\nOk :"))-1);
        }
        // Display output in UI
        ui->Console_Output_TextEdit->append(g_qtimeObj->currentTime().toString() + ": ");
        ui->Console_Output_TextEdit->append(g_ReadBuffer);
    }
}

// Update current temperature information
void MainWindow::UpdateTemp(){
    SerialCommand("temp",1);
    //    cout << "start" << endl;
    //    cout << g_ReadBuffer << endl;
    QString str = g_ReadBuffer;
    //    cout << str.size() << endl;
    //    if(str.size() < 118)
    //        qDebug() << str;
    QString component;
    int i = 0, j = 0, length = 0;
    if(str.at(0) == 'T'){
        for(int k = 0; k < 9; k++){
            component = "[";
            i = str.indexOf(component,j);
            component = "]";
            j = str.indexOf(component,i);
            if(j < i)
                j = i;
            length = j - i;
            if(length == 1)
                length++;
            component = str.mid(i+1,length-1);
            if((component.at(0) != 'T') && (component.at(0) != '<')){
                g_temp_value[k] = str.mid(i+1,length-1);
            }
        }
        ui->Temp_CCD_Value->setText(g_temp_value[0] + "°C");
        ui->Temp_CPU_Value->setText(g_temp_value[1] + "°C");
        ui->Temp_POWER_Value->setText(g_temp_value[2] + "°C");
        ui->Temp_BIAS_Value->setText(g_temp_value[3] + "°C");
        ui->Temp_WATER_Value->setText(g_temp_value[4] + "°C");
        ui->Temp_LEFT_Value->setText(g_temp_value[5] + "°C");
        ui->Temp_RIGHT_Value->setText(g_temp_value[6] + "°C");
        g_temp_value[7] = QString::number(g_temp_value[7].toDouble()/10);
        ui->Temp_SET_Value->setText(g_temp_value[7] + "°C");
        g_temp_value[8] = QString::number(g_temp_value[8].toDouble()/1000);
        ui->Temp_CoolingPower_Value->setText(g_temp_value[8] + "W");
    }else if(str.at(0) == '<'){
        for(int k = 0; k < 10; k++){
            component = "[";
            i = str.indexOf(component,j);
            component = "]";
            j = str.indexOf(component,i);
            if(j < i)
                j = i;
            length = j - i;
            if(length == 1)
                length++;
            component = str.mid(i+1,length-1);
            if(component.at(0) != '<'){
                g_temp_value[k] = str.mid(i+1,length-1);
            }
        }
        ui->Temp_CCD_Value->setText(g_temp_value[0] + "°C");
        ui->Temp_CPU_Value->setText(g_temp_value[1] + "°C");
        ui->Temp_POWER_Value->setText(g_temp_value[2] + "°C");
        ui->Temp_BIAS_Value->setText(g_temp_value[3] + "°C");
        ui->Temp_WATER_Value->setText(g_temp_value[4] + "°C");
        ui->Temp_LEFT_Value->setText(g_temp_value[5] + "°C");
        ui->Temp_RIGHT_Value->setText(g_temp_value[6] + "°C");
        g_temp_value[7] = QString::number(g_temp_value[7].toDouble()/10);
        ui->Temp_SET_Value->setText(g_temp_value[7] + "°C");
        g_temp_value[9] = QString::number(g_temp_value[9].toDouble()/1000);
        ui->Temp_CoolingPower_Value->setText(g_temp_value[9] + "W");
    }
}

// Get system current time
void MainWindow::GetTime(){
    QDateTime now = QDateTime::currentDateTime();
    QString now_str = now.toString("yyyy.MM.dd hh:mm:ss ddd");
    ui->Time_Value->setText(now_str);
}

// Zoom image
void MainWindow::ZoomImage(){
    // Zoom = register
    if(!g_zoom_full){
        // Get register image
        int offset_x = 0;
        int offset_y = 0;
        int i = 0;
        offset_x = g_zoom_id_register % (g_imgsize / g_mask_x) * g_mask_x;
        offset_y = g_mask_y * (int(g_zoom_id_register / (g_imgsize / g_mask_x)));
        for(int y = 0; y < g_mask_y; y++){
            for(int x = 0; x < g_mask_x; x++){
                g_register_zoom_8bits[y*g_mask_x+x] = g_imageNormal8bits[(offset_y * g_imgsize + offset_x) + x];
                g_register_zoom_vector[y*g_mask_x+x] = g_imageNormal[(offset_y * g_imgsize + offset_x) + x];
                i++;
            }
            offset_y++;
        }
        *g_zoom_image = QImage(60,120,QImage::Format_RGB888);
        for (int pos=0,i=0; i < g_mask_pixel; pos+=3,i++)
        {
            unsigned char PixVal = 0;
            short RawPixel= g_register_zoom_8bits[i];
            if (RawPixel >= 0xFF) //if pixel is above White point
                PixVal = 0xFF;//8 bit value is 255
            else if (RawPixel <= 0)//if below Black
                PixVal = 0;//value is 0
            else{
                PixVal = static_cast<unsigned char>(static_cast<int>(RawPixel));
            }
            g_zoom_image->bits()[pos] = PixVal;
            g_zoom_image->bits()[pos+1] = PixVal;
            g_zoom_image->bits()[pos+2] = PixVal;
        }
        g_scene_zoom->clear();
        g_scene_zoom->addPixmap(QPixmap::fromImage(g_zoom_image->scaled(240,480,Qt::IgnoreAspectRatio,Qt::FastTransformation)));
    }
    // Zoom = full
    if(g_zoom_full){
        *g_zoom_image = *g_image;
        g_scene_zoom->clear();
        g_scene_zoom->addPixmap(QPixmap::fromImage(g_zoom_image->scaled(960,960,Qt::IgnoreAspectRatio,Qt::FastTransformation)));
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        int pos_x = event->windowPos().x();
        int pos_y = event->windowPos().y();
        int rect_x = ui->graphicsView->pos().x()+1;
        int rect_y = ui->graphicsView->pos().y()+1;
        if(pos_x > rect_x && pos_x <= rect_x+g_mask_x){
            if(pos_y > rect_y && pos_y <= rect_y+g_mask_y){
                g_zoom_id_register = 0;
                ZoomImage();
            }
            if(pos_y > rect_y+g_mask_y && pos_y <= rect_y+2*g_mask_y){
                g_zoom_id_register = 4;
                ZoomImage();
            }
        }
        if(pos_x > rect_x+g_mask_x && pos_x <= rect_x+2*g_mask_x){
            if(pos_y > rect_y && pos_y <= rect_y+g_mask_y){
                g_zoom_id_register = 1;
                ZoomImage();
            }
            if(pos_y > rect_y+g_mask_y && pos_y <= rect_y+2*g_mask_y){
                g_zoom_id_register = 5;
                ZoomImage();
            }
        }
        if(pos_x > rect_x+2*g_mask_x && pos_x <= rect_x+3*g_mask_x){
            if(pos_y > rect_y && pos_y <= rect_y+g_mask_y){
                g_zoom_id_register = 2;
                ZoomImage();
            }
            if(pos_y > rect_y+g_mask_y && pos_y <= rect_y+2*g_mask_y){
                g_zoom_id_register = 6;
                ZoomImage();
            }
        }
        if(pos_x > rect_x+3*g_mask_x && pos_x <= rect_x+2+4*g_mask_x){
            if(pos_y > rect_y && pos_y <= rect_y+g_mask_y){
                g_zoom_id_register = 3;
                ZoomImage();
            }
            if(pos_y > rect_y+g_mask_y && pos_y <= rect_y+2*g_mask_y){
                g_zoom_id_register = 7;
                ZoomImage();
            }
        }
    }
}

// For 4k display, sampling method: Display value for each 17 pixels to output a 240*240 matrix
vector<unsigned short> MainWindow::Sampling4k(const vector<unsigned short> img)
{
    vector<unsigned short> ret;
    for(int i = 0; i < 17360640; i++){
        int x = i % 8448;
        int y = i / 8448;
        if(x > 8144)
            continue;
        if(y > 2024)
            continue;
        if(x % 17  == 0 && y % 17 == 0){
            ret.push_back(img[i]);
        }
    }
    return ret;
}

// For 4k display, mega pixel method: Display the mean value for each 17 pixel square to output 240*240 matrix
vector<unsigned short> MainWindow::MegaPixel4k(const vector<unsigned short> img)
{
    vector<unsigned short> ret(240*240,0);
    vector<vector <unsigned int>> square_val(240*240,vector<unsigned int>(17*17,0));
    vector<int> square_count(240*240,0);
    for(int i = 0; i < 8448*2055; i++){
        int x = i % 8448;
        int y = i / 8448;
        if(x > 8144)
            continue;
        if(y > 2024)
            continue;
        int mx = x / 17;
        int my = y / 17;
        int mindex = mx+my*480;
        square_val[mindex][square_count[mindex]] = img[i];
        square_count[mindex]++;
    }
    g_qtimeObj->start();
    for(int i = 0; i < 240*240; i++){
        ret[i] = g_maths.Median(square_val[i]);
//        qDebug() << "Time used: " << g_qtimeObj->elapsed() << "s";
//        ret[i] = g_maths.Mean(square_val[i]);
    }
    return ret;
}

void MainWindow::test2()
{
    // Create 16-bits MAT
    //    Mat img(2055,8448,CV_16UC1,Scalar(0));
    //    for (int j = 0; j < img.rows; j++){
    //        for(int i = 0; i < img.cols; i++){
    //            //            img.at<ushort>(j,i) = (img_fullsize[j*8448+i]) & 0xff;
    //            img.at<ushort>(j,i) = (g_img4K[j*8448+i]) & 0xff;
    //        }
    //    }
    //    string filepath = "/acqui/png/4kimgtest.png";
    //    imwrite(filepath,img);
    //    cout << filepath << " saved." << endl;
    if(g_disp4k_show){
        *g_image4K = QImage(8448,2055,QImage::Format_RGB888);
        for (int pos=0,i=0; i < 2055*8448; pos+=3,i++)
        {
            unsigned char PixRed=0, PixGreen=0, PixBlue=0;
            short RawPixel= static_cast<short>(g_img4K[i] & 0xff);
            if (RawPixel >= 0xFF) //if pixel is above White point
                PixRed= PixGreen= PixBlue= 0xFF;//8 bit value is 255
            else if (RawPixel <= 0)//if below Black
                PixRed= PixGreen= PixBlue = 0;//value is 0
            else{
                PixRed= PixGreen= PixBlue= static_cast<unsigned char>(static_cast<int>(RawPixel));
            }
            g_image4K->bits()[pos] = PixRed;
            g_image4K->bits()[pos+1] = PixGreen;
            g_image4K->bits()[pos+2] = PixBlue;
        }
        g_scene_4k->clear();
        g_scene_4k->addPixmap(QPixmap::fromImage(*g_image4K));
    }

    vector<unsigned short> img_disp_fullsize(8448*2055,0);
    for(int i = 0; i < 8448*2055; i++){
        //        img_disp_fullsize[i] = img_fullsize[i] & 0xff;
        img_disp_fullsize[i] = g_img4K[i] & 0xff;
    }
    //    g_img4K_disp = Sampling4k(img_disp_fullsize);
    if(g_sampling4k){
        g_img4K_disp = Sampling4k(img_disp_fullsize);
    }
    if(g_megapixel4k){
        g_img4K_disp = MegaPixel4k(img_disp_fullsize);
    }
    display_4K(g_img4K_disp);

}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int pos_x = mouseEvent->windowPos().x();
        int pos_y = mouseEvent->windowPos().y();
        int rect_x = ui->graphicsView->pos().x()+1;
        int rect_y = ui->graphicsView->pos().y()+1;
        if(pos_x > rect_x && pos_x <= rect_x+2+4*g_mask_x && pos_y > rect_y && pos_y <= rect_y+2*g_mask_y)
            this->setCursor(Qt::PointingHandCursor);
        else
            this->setCursor(Qt::ArrowCursor);
    }
    return false;
}

void MainWindow::onTimeOut()
{
    UpdateTemp();
    GetTime();
}
/*================= FUNCTIONS END ======================================*/

/*================= QT GUI BUTTONS START ======================================*/

// Exit button
void MainWindow::on_Exit_PB_clicked()
{
    g_run_state = false;
    QApplication::quit();
}

// Define BP, WP for a set of images
void MainWindow::on_Preset_PB_clicked()
{
    // Start Acquisitions for this channel.
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    if(g_status != MC_OK){
        test_error();
    }

    //    g_status= McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_IDLE);

    ocam2_descramble(g_id, &g_number, g_imageNormal, g_imageRawNormal);
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        g_imageNormal[i] = g_imageNormal[i] & 0x3fff;
    }

    g_BP = OCAM2_PIXEL_MAX_VAL;
    g_WP = OCAM2_PIXEL_MIN_VAL;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
        short RawPixel= g_imageNormal[i];

        if (RawPixel < g_BP)
            g_BP=RawPixel;

        if (g_WP <RawPixel)
            g_WP=RawPixel;

        if (g_BP == g_WP)//prevent equality between White and Black clipping values
            g_WP++;
    }
    qDebug() << "BP: "  << g_BP;
    qDebug() << "WP: "  << g_WP;
    ui->BP_Slider->setValue(g_BP);
    ui->WP_Slider->setValue(g_WP);
    threshold_function(short(ui->Threshold_Slider->value()));
    pixel_correction(g_imageNormalThresh,g_imageNormal8bits);
    display(g_imageNormal8bits);
}

// Acquire Dark Images
void MainWindow::on_Dark_PB_clicked()
{
    // Total frames acquired
    g_surfacebuffercount = 0;
    g_callback_mode = 0;
    unsigned int darkframes = 2000;
    g_buffersize = darkframes;
    // g_surfacebuffer 2000*127776
    g_surfacebuffer = new unsigned char *[darkframes];
    for(int i = 0; i < darkframes; i++){
        g_surfacebuffer[i] = new unsigned char[g_sizeX*g_sizeY];
    }
    g_qtimeObj->start();
    // Start Acquisitions for this channel.
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    while(g_surfacebuffercount < darkframes-1);
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    if(g_status != MC_OK){
        test_error();
    }
    ui->op_progressBar->setMaximum(OCAM2_PIXELS_IMAGE_NORMAL-1);
    // Allocate memory: imagedark 2000*57600
    short **imagedark = new short*[darkframes];
    for(unsigned long i = 0; i < darkframes; i++){
        imagedark[i] = new short[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    for(int k = 0; k < darkframes; k++){
        // Merge two 8-bit char to a 16-bit short
        for(int j = 0;j<OCAM2_PIXELS_RAW_NORMAL;j++){
            g_imageRawNormal[j] = (static_cast<short>((g_surfacebuffer[k][j*2+1])<<8)) | (g_surfacebuffer[k][j*2]);
        }
        ocam2_descramble(g_id, &g_number, g_imageNormal, g_imageRawNormal);
        // Convert to 14 bits data & assign to local buffer
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            g_imageNormal[i] = g_imageNormal[i] & 0x3fff;
            imagedark[k][i] = g_imageNormal[i];
        }
    }
    // Calculate mean image
    short pixelarray[darkframes]; // short[2000]
    // For each pixel for each frame
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
        for(unsigned long j = 0; j < darkframes; j++){
            pixelarray[j] = imagedark[j][i];
        }
        // Calculate mean for 1 pixel & assign to g_imageNormalDark
        g_imageNormalDark[i] = g_maths.Mean(short2float(pixelarray,darkframes));
        ui->op_progressBar->setValue(i);
    }
    // σ²Read
    float sigma2Read = 0;
    for(int j = 0; j < darkframes - 1; j++){
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            sigma2Read += pow(static_cast<float>(imagedark[j][i] - imagedark[j+1][i]), 2);
        }
    }
    sigma2Read = sigma2Read /( 2 * (darkframes - 1) * OCAM2_PIXELS_IMAGE_NORMAL);
    cout << dec << "σ²Read = " << sigma2Read << endl;
    // Get current gain
    SerialCommand("gain",1);
    QString str = g_ReadBuffer;
    qDebug() << str;
    QString component;
    int i = 0, j = 0, length = 0;
    // Interface 0
    if(str.at(0) == '<'){
        component = "[";
        i = str.indexOf(component);
        component = "]";
        j = str.indexOf(component,i);
        length = j - i;
        component = str.mid(i+1,length-1);
    }
    // Interface 1
    else{
        component = "o ";
        i = str.indexOf(component);
        i++;
        component = "\n\n\nOk :";
        j = str.indexOf(component,i);
        length = j - i;
        component = str.mid(i+1,length-1);
    }
    g_gain_value = component.toInt();
    cout << "Gain = " << g_gain_value << endl;
    ui->Gain_Label->setText("Gain = " + component);
    // Save dark image
    QString filename = "/acqui/dark/dark_"+component+".dat";
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        QDataStream out(&file);
        out << static_cast<qint16>(0xff);
        out.setVersion(QDataStream::Qt_5_11);
        out << static_cast<qint16>(g_gain_value);
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            out << static_cast<qint16>(g_imageNormalDark[i]);
        }
        out << static_cast<qreal>(sigma2Read);
    }
    file.close();
    cout << "Dark image saved." << endl;

    //    // OpenCV
    //    Mat img(240,240,CV_16UC1,Scalar(0));
    //    for (int j = 0; j < img.rows; j++){
    //        for(int i = 0; i < img.cols; i++){
    //            img.at<ushort>(j,i) = mean_img[j*240+i];
    //        }
    //    }
    //    imshow("dark mean", img);
    //    imwrite("/home/gcai/LAM/Qt/Ocam/png/dark.png",img);

    // Free memory
    for(unsigned long i = 0; i < darkframes; i++)
        delete[] imagedark[i];
    delete[] imagedark;
    ChangeSection();
    short img_display[OCAM2_PIXELS_IMAGE_NORMAL];
    pixel_correction(g_imageNormalDark,img_display);
    display(img_display);
    // Min & Max
    short min = 16383, max = 0;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
        if(g_imageNormalDark[i] < min)
            min = g_imageNormalDark[i];
        if(g_imageNormalDark[i] > max)
            max = g_imageNormalDark[i];
    }
    ui->Dark_Max_Label->setText("Max: " + QString::number(max));
    ui->Dark_Min_Label->setText("Min: " + QString::number(min));
    qDebug() << "Time used: " << g_qtimeObj->elapsed()/1000.0 << "s";
    ChangeSection();
}

// Flat operation
void MainWindow::on_Flat_PB_clicked()
{
    // Total frames acquired
    g_surfacebuffercount = 0;
    g_callback_mode = 0;
    unsigned int flatframes = 2000;
    g_buffersize = flatframes;
    // g_surfacebuffer 2000*127776
    g_surfacebuffer = new unsigned char *[flatframes];
    for(int i = 0; i < flatframes; i++){
        g_surfacebuffer[i] = new unsigned char[g_sizeX*g_sizeY];
    }
    g_qtimeObj->start();
    // Start Acquisitions for this channel.
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    while(g_surfacebuffercount < flatframes-1);
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    if(g_status != MC_OK){
        test_error();
    }
    ui->op_progressBar->setMaximum(OCAM2_PIXELS_IMAGE_NORMAL-1);
    // Allocate memory: imageflat 2000*57600
    short **imageflat = new short*[flatframes];
    for(unsigned long i = 0; i < flatframes; i++){
        imageflat[i] = new short[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    for(int k = 0; k < flatframes; k++){
        // Merge two 8-bit char to a 16-bit short
        for(int j = 0;j<OCAM2_PIXELS_RAW_NORMAL;j++){
            g_imageRawNormal[j] = (static_cast<short>((g_surfacebuffer[k][j*2+1])<<8)) | (g_surfacebuffer[k][j*2]);
        }
        ocam2_descramble(g_id, &g_number, g_imageNormal, g_imageRawNormal);
        // Convert to 14 bits data & assign to local buffer && substract the dark
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            g_imageNormal[i] = g_imageNormal[i] & 0x3fff;
            imageflat[k][i] = g_imageNormal[i] - g_imageNormalDark[i];
            if(imageflat[k][i] < 0)
                imageflat[k][i] = 0;
        }
    }
    /*    // Separate each register
//    int nb_register = 8;
//    vector<vector< vector <short>>> img_register;
//    img_register.resize(flatframes);
//    for(int i = 0; i < flatframes; i++){
//        img_register[i].resize(nb_register);
//        for(int j = 0; j < nb_register; j++){
//            img_register[i][j].resize(g_mask_pixel);
//        }
//    }
//    int offset_x = 0;
//    int offset_y = 0;
//    for(unsigned long k = 0; k < flatframes; k++){
//        for(int j = 0; j < nb_register; j++){
//            int i = 0;
//            offset_x = j % (g_imgsize / g_mask_x) * g_mask_x;
//            offset_y = g_mask_y * (int(j / (g_imgsize / g_mask_x)));
//            for(int y = 0; y < g_mask_y; y++){
//                for(int x = 0; x < g_mask_x; x++){
//                    img_register[k][j][y*g_mask_x+x] = imageflat[k][(offset_y * g_imgsize + offset_x) + x];
//                    i++;
//                }
//                offset_y++;
//            }
//        }
//    } */
    // Calculate mean flat image
    short pixelarray[flatframes]; // short[2000]
    // For each pixel for each frame
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
        for(unsigned long j = 0; j < flatframes; j++){
            pixelarray[j] = imageflat[j][i];
        }
        // Calculate mean for 1 pixel & assign to g_imageNormalFlat
        g_imageNormalFlat[i] = g_maths.Mean(short2float(pixelarray,flatframes));
        ui->op_progressBar->setValue(i);
    }
    // Get current gain
    SerialCommand("gain",1);
    QString str = g_ReadBuffer;
    qDebug() << str;
    QString component;
    int i = 0, j = 0, length = 0;
    // Interface 0
    if(str.at(0) == '<'){
        component = "[";
        i = str.indexOf(component);
        component = "]";
        j = str.indexOf(component,i);
        length = j - i;
        component = str.mid(i+1,length-1);
    }
    // Interface 1
    else{
        component = "o ";
        i = str.indexOf(component);
        i++;
        component = "\n\n\nOk :";
        j = str.indexOf(component,i);
        length = j - i;
        component = str.mid(i+1,length-1);
    }
    g_gain_value = component.toInt();
    cout << "Gain = " << g_gain_value << endl;
    ui->Gain_Label->setText("Gain = " + component);
    // Save flat image
    QString filename = "/acqui/flat/flat_"+component+".dat";
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        QDataStream out(&file);
        out << static_cast<qint16>(0xff);
        out.setVersion(QDataStream::Qt_5_11);
        out << static_cast<qint16>(g_gain_value);
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            out << static_cast<qint16>(g_imageNormalFlat[i]);
        }
    }
    file.close();
    cout << "Flat image saved." << endl;

    //    // OpenCV
    //    Mat img(240,240,CV_16UC1,Scalar(0));
    //    for (int j = 0; j < img.rows; j++){
    //        for(int i = 0; i < img.cols; i++){
    //            img.at<ushort>(j,i) = mean_img[j*240+i];
    //        }
    //    }
    //    imshow("dark mean", img);
    //    imwrite("/home/gcai/LAM/Qt/Ocam/png/dark.png",img);

    // Free memory
    for(unsigned long i = 0; i < flatframes; i++)
        delete[] imageflat[i];
    delete[] imageflat;
    ChangeSection();
    short img_display[OCAM2_PIXELS_IMAGE_NORMAL];
    pixel_correction(g_imageNormalFlat,img_display);
    display(img_display);
    // Min & Max
    short min = 16383, max = 0;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
        if(g_imageNormalFlat[i] < min)
            min = g_imageNormalFlat[i];
        if(g_imageNormalFlat[i] > max)
            max = g_imageNormalFlat[i];
    }
    cout << dec << "Flat max = " << max << endl << "Flat min = " << min << endl;
    qDebug() << "Time used: " << g_qtimeObj->elapsed()/1000.0 << "s";
    ChangeSection();
}

// Load Flat Button
void MainWindow::on_LoadFlat_PB_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Open file",g_data_path+"flat/","Files (*.dat)");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
    }else{
        QDataStream in(&file);
        // Read and check the header
        qint16 magic;
        in >> magic;
        if(magic != 0xff)
            QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
        else{
            in.setVersion(QDataStream::Qt_5_11);
            qint16 data;
            in >> data;
            g_gain_value = data;
            ui->Gain_Label->setText("Gain = " + QString::number(g_gain_value));
            for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
                in >> data;
                g_imageNormalFlat[i] = data;
            }
        }
    }
    file.close();
    // Min & Max
    short min = 16383, max = 0;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
        if(g_imageNormalFlat[i] < min)
            min = g_imageNormalFlat[i];
        if(g_imageNormalFlat[i] > max)
            max = g_imageNormalFlat[i];
    }
    cout << dec << "Flat max = " << max << endl << "Flat min = " << min << endl;
    cout << "Flat image loaded." << endl;
}

// Display Dark Image Button
void MainWindow::on_Darkimage_PB_clicked()
{
    short darkimage[OCAM2_PIXELS_IMAGE_NORMAL];
    pixel_correction(g_imageNormalDark,darkimage);
    display(darkimage);
    // Create 16-bits MAT
    Mat img(g_imgsize,g_imgsize,CV_16UC1,Scalar(0));
    for (int j = 0; j < img.rows; j++){
        for(int i = 0; i < img.cols; i++){
            img.at<ushort>(j,i) = static_cast<unsigned short>(g_imageNormalDark[j*240+i]);
        }
    }
    short imgtmp[OCAM2_PIXELS_IMAGE_NORMAL];
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        imgtmp[i] = g_imageNormalDark[i];
    }
    //    imshow(to_string(g_num), img);
    string filepath = "/acqui/png/dark.png";
    imwrite(filepath,img);
    cout << filepath << " saved." << endl;
    g_meanvalue = g_maths.Mean(short2float(imgtmp,OCAM2_PIXELS_IMAGE_NORMAL));
    g_median = g_maths.Median(short2float(imgtmp,OCAM2_PIXELS_IMAGE_NORMAL));
    g_variance = g_maths.Variance(short2float(imgtmp,OCAM2_PIXELS_IMAGE_NORMAL));
    g_SD = g_maths.StandardDeviation(short2float(imgtmp,OCAM2_PIXELS_IMAGE_NORMAL));
    g_SNR = g_meanvalue / g_SD;
    ui->Mean_Label->setText("Mean: " + QString::number(static_cast<double>(g_meanvalue)));
    ui->Median_Label->setText("Median: " + QString::number(g_median));
    ui->Variance_Label->setText("Variance: " + QString::number(static_cast<double>(g_variance)));
    ui->StandardDeviation_Label->setText("Ecart-type: " + QString::number(static_cast<double>(g_SD)));
    ui->SNR_Label->setText("SNR: " + QString::number(static_cast<double>(g_SNR)));
    ui->Statistics_PB->setEnabled(false);
    cout << dec << "Mean: " << g_meanvalue << "    Median: " << g_median <<
            "    Variance: " << g_variance << " Ecart-type: " << g_SD << " SNR: " << g_SNR << endl;
}

// Load Dark Button
void MainWindow::on_LoadDark_PB_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Open file",g_data_path+"dark/","Files (*.dat)");
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
    }else{
        QDataStream in(&file);
        // Read and check the header
        qint16 magic;
        in >> magic;
        if(magic != 0xff)
            QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
        else{
            in.setVersion(QDataStream::Qt_5_11);
            qint16 data;
            in >> data;
            g_gain_value = data;
            ui->Gain_Label->setText("Gain = " + QString::number(g_gain_value));
            for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
                in >> data;
                g_imageNormalDark[i] = data;
            }
            qreal sigma;
            in >> sigma;
            g_Sigma2Read = sigma;
        }
    }
    file.close();
    // Min & Max
    short min = 16383, max = 0;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
        if(g_imageNormalDark[i] < min)
            min = g_imageNormalDark[i];
        if(g_imageNormalDark[i] > max)
            max = g_imageNormalDark[i];
    }
    ui->Dark_Max_Label->setText("Max: " + QString::number(max));
    ui->Dark_Min_Label->setText("Min: " + QString::number(min));
    cout << "Dark image loaded." << endl;
    cout << dec << "σ²Read = " << g_Sigma2Read << endl;
    cout << dec << "5σ = " << 5*sqrt(g_Sigma2Read) << endl;
}

// Load Bias Button
void MainWindow::on_Load_Bias_PB_clicked()
{
    QString str = QString::number(ui->Gain_SpinBox->value());
    // Load Flat
    QString fileName = g_data_path+"flat/flat_" + str + ".dat";
    QFile file_flat(fileName);
    if(!file_flat.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
    }else{
        QDataStream in(&file_flat);
        // Read and check the header
        qint16 magic;
        in >> magic;
        if(magic != 0xff)
            QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
        else{
            in.setVersion(QDataStream::Qt_5_11);
            qint16 data;
            in >> data;
            g_gain_value = data;
            ui->Gain_Label->setText("Gain = " + QString::number(g_gain_value));
            for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
                in >> data;
                g_imageNormalFlat[i] = data;
            }
        }
    }
    file_flat.close();
    cout << "Flat image loaded." << endl;
    // Load Dark
    fileName = g_data_path+"dark/dark_" + str + ".dat";
    QFile file_dark(fileName);
    if(!file_dark.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
    }else{
        QDataStream in(&file_dark);
        // Read and check the header
        qint16 magic;
        in >> magic;
        if(magic != 0xff)
            QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
        else{
            in.setVersion(QDataStream::Qt_5_11);
            qint16 data;
            in >> data;
            g_gain_value = data;
            ui->Gain_Label->setText("Gain = " + QString::number(g_gain_value));
            for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
                in >> data;
                g_imageNormalDark[i] = data;
            }
            qreal sigma;
            in >> sigma;
            g_Sigma2Read = sigma;
        }
    }
    file_dark.close();
    cout << "Dark image loaded." << endl;
}

// Snap Shot Button
void MainWindow::on_Snap_shot_PB_clicked()
{
    if(!g_test4K){
        AcquireImages();
        display(g_imageNormal8bits);
        ZoomImage();
    }else{
        g_4kacqui = false;
        cout << "Acquire 4K snap shot" << endl;
        AcquireImages4K();
        display_4K(g_img4K_disp);
        //        display_4K(g_img4Kraw_vector);
    }
    cout << "Snap shot finish" << endl;
    QString imagenum = QString::number(g_num-1) + ".dat";
    ui->ImageName_Label->setText(imagenum);
    ui->Statistics_PB->setEnabled(true);
}

// Run Button
void MainWindow::on_Run_PB_clicked(bool checked)
{
    QCoreApplication::processEvents();
    if(checked==true){
        g_run_state = true;
        ui->Run_PB->setText("Stop");
        ui->BufferAcquire_PB->setEnabled(false);
        cout << "Acquiring..." << endl;
        while(g_run_state == true){
            if(!g_test4K){
                AcquireImages();
                display(g_imageNormal8bits);
                ZoomImage();
                QString imagenum = QString::number(g_num-1) + ".dat";
                ui->ImageName_Label->setText(imagenum);
                QTest::qWait(0);
            }else{
                g_4kacqui = false;
                AcquireImages4K();
                QString imagenum = QString::number(g_num-1) + ".dat";
                ui->ImageName_Label->setText(imagenum);
                QTest::qWait(0);
            }
        }
    }
    if(checked == false){
        ui->Run_PB->setText("Run");
        ui->BufferAcquire_PB->setEnabled(true);
        g_run_state = false;
        cout << "Stopped..." << endl;
    }
    ui->Statistics_PB->setEnabled(true);
}

// Gain Histrogram Button
void MainWindow::on_Gain_Histo_PB_clicked()
{
    // Total frames acquired
    g_surfacebuffercount = 0;
    g_callback_mode = 0;
    unsigned int frames = 2000;
    ui->op_progressBar->setMaximum(int(frames));
    g_buffersize = frames;
    g_surfacebuffer = new unsigned char *[frames];
    for(int i = 0; i < int(frames); i++){
        g_surfacebuffer[i] = new unsigned char[static_cast<unsigned long>(g_sizeX*g_sizeY)];
    }
    g_qtimeObj->start();
    // Start Acquisitions for this channel.
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    while(g_surfacebuffercount < int(frames)-1);
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    if(g_status != MC_OK){
        test_error();
    }
    // Allocate memory
    short **image = new short*[frames];
    for(unsigned long i = 0; i < frames; i++){
        image[i] = new short[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    for(int k = 0; k < int(frames); k++){
        // Merge two 8-bit char to a 16-bit short
        for(int j = 0;j<OCAM2_PIXELS_RAW_NORMAL;j++){
            g_imageRawNormal[j] = (static_cast<short>((g_surfacebuffer[k][j*2+1])<<8)) | (g_surfacebuffer[k][j*2]);
        }
        ocam2_descramble(g_id, &g_number, g_imageNormal, g_imageRawNormal);
        // Convert to 14 bits data
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            g_imageNormal[i] = g_imageNormal[i] & 0x3fff;
            image[k][i] = g_imageNormal[i];
            // Flat image suppression
            image[k][i] = ((image[k][i] - g_imageNormalFlat[i]) < 0) ? 0 : (image[k][i] - g_imageNormalFlat[i]);
            // Dark image suppression
            image[k][i] = ((image[k][i] - g_imageNormalDark[i]) < 0) ? 0 : (image[k][i] - g_imageNormalDark[i]);
        }
    }
    cout << "Grab frames ok" << endl;
    // Separate each register
    int nb_register = 8;
    vector<vector< vector <short>>> img_register;
    img_register.resize(frames);
    for(unsigned long i = 0; i < frames; i++){
        img_register[i].resize(static_cast<unsigned long>(nb_register));
        for(unsigned long j = 0; j < static_cast<unsigned long>(nb_register); j++){
            img_register[i][j].resize(static_cast<unsigned long>(g_mask_pixel));
        }
    }
    int offset_x = 0;
    int offset_y = 0;
    for(unsigned long k = 0; k < frames; k++){
        for(unsigned long j = 0; j < static_cast<unsigned long>(nb_register); j++){
            unsigned long i = 0;
            offset_x = j % (g_imgsize / g_mask_x) * g_mask_x;
            offset_y = g_mask_y * (int(j / (g_imgsize / g_mask_x)));
            for(int y = 0; y < g_mask_y; y++){
                for(int x = 0; x < g_mask_x; x++){
                    img_register[k][j][y*g_mask_x+x] = image[k][(offset_y * g_imgsize + offset_x) + x];
                    i++;
                }
                offset_y++;
            }
        }
    }
    cout << "Separate register ok" << endl;
    // Output Threshold T
    vector<vector< vector <double>>> fraction_register;
    fraction_register.resize(frames);
    for(int i = 0; i < frames; i++){
        fraction_register[i].resize(nb_register);
        for(int j = 0; j < nb_register; j++){
            fraction_register[i][j].resize(g_n_t);
        }
    }
    for(int k = 0; k < frames; k++){
        for(int j = 0; j < nb_register; j++){
            for(int i = 0; i < g_n_t; i++){
                fraction_register[k][j][i] = 0;
            }
        }
    }
    for(int k = 0; k < frames; k++){
        for(int t = 0; t < g_n_t; t++){
            for(int j = 0; j < nb_register; j++){
                int n_above_T = 0;
                for(int i = 0; i < g_mask_pixel; i++){
                    if(img_register[k][j][i] > t * g_t_length)
                        n_above_T++;
                }
                fraction_register[k][j][t] = n_above_T;
            }
        }
        ui->op_progressBar->setValue(k+1);
        //        if(k % 100 == 0)
        //            cout << dec << k << " / " << frames << endl;
    }
    cout << "Threshold ok" << endl;
    // Log
    vector<vector< vector <double>>> log_fraction_register;
    log_fraction_register.resize(frames);
    for(int i = 0; i < frames; i++){
        log_fraction_register[i].resize(nb_register);
        for(int j = 0; j < nb_register; j++){
            log_fraction_register[i][j].resize(g_n_t);
        }
    }
    for(int k = 0; k < frames; k++){
        for(int j = 0; j < nb_register; j++){
            for(int i = 0; i < g_n_t; i++){
                log_fraction_register[k][j][i] = 0;
            }
        }
    }
    for(int k = 0; k < frames; k++){
        for(int j = 0; j < nb_register; j++){
            for(int i = 0; i < g_n_t; i++){
                if(fraction_register[k][j][i] == 0)
                    fraction_register[k][j][i] = g_mask_pixel;
                log_fraction_register[k][j][i] = log(fraction_register[k][j][i]/double(g_mask_pixel));
            }
        }
    }
    cout << "Log histogram ok" << endl;
    // Mean of 2000 images
    for(int j = 0; j < nb_register; j++){
        for(int i = 0; i < g_n_t; i++){
            double sum = 0;
            for(int k = 0; k < frames; k++){
                sum += log_fraction_register[k][j][i];
            }
            g_mean_histo_register[j][i] = sum/frames;
        }
        cout << j+1 << " / " << nb_register << endl;
    }
}

// Zoom Button
void MainWindow::on_Zoom_PB_clicked()
{
    if(!g_zoom_show){
        Zoomer_win = new Zoomer;
        Zoomer_win->show();
        g_zoom_show = true;
    }
}

// Display 4K
void MainWindow::on_Display_4K_PB_clicked()
{
    if(!g_disp4k_show){
        Disp4K_win = new Display4K;
        Disp4K_win->show();
        g_disp4k_show = true;
    }
}

// Single Save Button
void MainWindow::on_SingleSave_PB_clicked()
{
    QString filename = "/acqui/data/"+QString::number(g_num-1) + ".dat";
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        QDataStream out(&file);
        out << (qint16)0xff;
        out.setVersion(QDataStream::Qt_5_11);
        out << (qint32)g_num;
        out << (qint32)(g_BP);
        out << (qint32)(g_WP);
        if(g_afterthresh >= ((OCAM2_PIXELS_IMAGE_NORMAL/2)-1) || g_afterthresh == 0){
            // Write a header with a "magic number" and a version
            QString str = "full";
            out << str;
            for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
                out << (qint16)(g_imageNormalThresh[i]);
            }
        }else{
            QString str = "thresh";
            out << str;
            out << (qint32)(g_afterthresh);
            for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
                if(g_imageNormalThresh[i] != g_BP){
                    out << (quint16)(i);
                    out << (qint16)(g_imageNormalThresh[i]);
                }
            }
        }
    }
    file.close();
    qDebug() << filename << "saved.";
}

// Single Load Button
void MainWindow::on_SingleLoad_PB_clicked()
{
    // reset imageNormal to 0;
    int loadnum = 0;
    QString fileName = QFileDialog::getOpenFileName(this,"Open file",g_data_path,"Files (*.dat)");
    if(fileName != ""){
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)){
            QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
        }else{
            QDataStream in(&file);
            // Read and check the header
            qint16 magic;
            in >> magic;
            if(magic != 0xff)
                QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
            else{
                in.setVersion(QDataStream::Qt_5_11);
                qint32 headnum;
                in >> headnum;
                loadnum = headnum;
                qint32 data;
                qint16 val;
                in >> data;
                g_BP = data;
                in >> data;
                g_WP = data;
                QString str;
                quint16 pos;
                in >> str;
                qDebug() << str;
                if(str == "full"){
                    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
                        in >> val;
                        g_imageNormalThresh[i] = val;
                        //                        qDebug() << val;
                    }
                }else if(str == "thresh"){
                    in >> data;
                    g_afterthresh = data;
                    //                    cout << dec << g_afterthresh << endl;
                    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
                        g_imageNormalThresh[i] = static_cast<short>(g_BP);
                    }
                    for(int i = 0; i < g_afterthresh; i++){
                        in >> pos;
                        in >> val;
                        g_imageNormalThresh[pos] = val;
                        //                        qDebug() << data;
                    }
                }
            }
        }
        file.close();
    }
    pixel_correction(g_imageNormalThresh,g_imageNormal8bits);
    display(g_imageNormal8bits);
    ui->Thresh_Label->setText("after thresh = " + QString::number(g_afterthresh));
    QString imagenum = QString::number(loadnum-1) + ".dat";
    ui->ImageName_Label->setText(imagenum);
    qDebug() << fileName << "loaded";
}

// Sequence Save Button
void MainWindow::on_SequenceSave_PB_clicked()
{
    qDebug() << "Start sequence save";
    // Get frames
    int frames = ui->Sequence_spinBox->value();
    ui->progressBar->setMaximum(frames);
    QString filename = "/acqui/data/frames"+QString::number(g_num_frames) + "_" + QString::number(frames) + ".dat";
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        QDataStream out(&file);
        // Write a header with a "magic number" and a version
        out << static_cast<qint16>(0xee);
        out.setVersion(QDataStream::Qt_5_11);
        out << static_cast<qint32>(frames);
        out << static_cast<qint32>(g_BP);
        out << static_cast<qint32>(g_WP);
        for(int j = 0; j < frames; j++){
            AcquireImages();
            // Save all pixels
            if(g_afterthresh >= ((OCAM2_PIXELS_IMAGE_NORMAL/3)-1) || g_afterthresh == 0){
                QString str = "full";
                out << str;
                for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
                    out << static_cast<qint16>(g_imageNormalThresh[i]);
                }
            }
            // Save only address
            else{
                QString str = "thresh";
                out << str;
                out << static_cast<qint32>(g_afterthresh);
                for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
                    if(g_imageNormalThresh[i] != g_BP){
                        out << static_cast<quint32>(i);
                        out << static_cast<qint16>(g_imageNormalThresh[i]);
                    }
                }
            }
            ui->progressBar->setValue(j);
        }
        ui->progressBar->setValue(ui->progressBar->maximum());
    }
    file.close();
    qDebug() << filename << "saved.";
    g_num_frames++;
}

// Sequence Load Button
void MainWindow::on_SequenceLoad_PB_clicked()
{
    long loadframes = 0;
    int loadg_afterthresh;
    QString fileName = QFileDialog::getOpenFileName(this,"Open file",g_data_path,"Files (*.dat)");
    if(fileName != ""){
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)){
            QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
        }else{
            QDataStream in(&file);
            // Read and check the header
            qint16 magic;
            in >> magic;
            if(magic != 0xee)
                QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
            else{
                in.setVersion(QDataStream::Qt_5_11);
                qint32 frames;
                in >> frames;
                loadframes = frames;
                ui->progressBar->setMaximum(loadframes);
                qint32 data;
                in >> data;
                g_BP = data;
                in >> data;
                g_WP = data;
                QString str;
                quint32 pos;
                qint16 val;
                for(int j = 0; j < loadframes; j++){
                    in >> str;
                    if(str == "full"){
                        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
                            in >> val;
                            g_imageNormalThresh[i] = val;
                        }
                    }else if(str == "thresh"){
                        in >> data;
                        loadg_afterthresh = data;
                        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
                            g_imageNormalThresh[i] = static_cast<short>(g_BP);
                        }
                        for(int i = 0; i < loadg_afterthresh; i++){
                            in >> pos;
                            in >> val;
                            g_imageNormalThresh[pos] = val;
                        }
                    }
                    pixel_correction(g_imageNormalThresh,g_imageNormal8bits);
                    display(g_imageNormal8bits);
                    ui->progressBar->setValue(j);
                    QTest::qWait(g_display_frequency);
                }
                ui->progressBar->setValue(ui->progressBar->maximum());
            }
        }
        file.close();
    }
    pixel_correction(g_imageNormalThresh,g_imageNormal8bits);
    display(g_imageNormal8bits);
    QString imagenum = QString::number(loadframes-1) + ".dat";
    ui->ImageName_Label->setText(imagenum);
    qDebug() << fileName << "loaded";
}

// Allocate Buffer
void MainWindow::on_BufferAllocate_PB_clicked()
{
    try {
        delete g_surfacebuffer_short;
        g_callback_mode = 0;
        g_qtimeObj->start();
        g_buffersize = ui->Sequence_spinBox->value();
        //        g_surfacebuffer = new unsigned char *[g_buffersize];
        g_surfacebuffer_short = new unsigned short *[g_buffersize];
        for(int i = 0; i < g_buffersize; i++){
            //            g_surfacebuffer[i] = new unsigned char[g_sizeX*g_sizeY];
            if(!g_test4K){
                g_surfacebuffer_short[i] = new unsigned short[OCAM2_PIXELS_IMAGE_NORMAL];
            }else{
                g_surfacebuffer_short[i] = new unsigned short[8448*2055];
            }
        }
        int t = g_qtimeObj->elapsed();
        qDebug() << "Time used: " << t/1000.0 << "s";
        //    for(int i = 0; i < frames; i++){
        //        for(int j = 0; j < g_sizeX*g_sizeY; j++){
        //            g_surfacebuffer[i][j] = 0;
        //        }
        //    }
    } catch (const char* msg) {
        cerr << msg << endl;
    }
}

// Acquire Raw Buffer
void MainWindow::on_BufferAcquire_PB_clicked()
{
    qDebug() << "Start acquiring image to buffer";
    g_callback_mode = 0;
    g_surfacebuffercount = 0;
    g_qtimeObj->start();
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    while(g_surfacebuffercount < g_buffersize-1);
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    cout << dec << g_surfacebuffercount << endl;
    int t = g_qtimeObj->elapsed();
    cout << dec << "surface buffer count = " << g_surfacebuffercount << endl;
    qDebug() << "...Done!";
    qDebug() << "Acquired" << QString::number(g_buffersize) << "images";
    // Check identical frames
    //    ChangeSection();
    //    int k = 0;
    //    for(int i = 0; i < g_buffersize - 1; i++){
    //        int p = 0;
    //        for(int j = 0; j < g_sizeX*g_sizeY; j++){
    //            if(g_surfacebuffer[i][j] != g_surfacebuffer[i+1][j]){
    //                p = 1;
    //            }
    //        }
    //        if( p == 0){
    //            cout << dec << i << " & " << i+1 << " same" << endl;
    //            k++;
    //        }
    //    }
    //    cout << dec << k << endl;
    qDebug() << "Time used: " << t/1000.0 << "s";
}

// Save Raw Buffer
void MainWindow::on_BufferSave_PB_clicked()
{
    cout << "save start" << endl;
    g_qtimeObj->start();
    qDebug() << "Start Buffer Saving";
    ui->progressBar->reset();
    ui->progressBar->setMaximum(g_buffersize-1);
    FILE *pFile;
    string filepath;
    if(!g_test4K){
        filepath = "/acqui/data/Rawframes" + to_string(g_buffersize) + "_frames.dat";
    }else{
        filepath = "/acqui/data/Rawframes_4k_" + to_string(g_buffersize) + "_frames.dat";
    }
    char * filename = new char [filepath.length()+1];
    std::strcpy(filename,filepath.c_str());
    pFile = fopen(filename,"wb");
    if(!g_test4K){
        // Allocate buffer for short
        short imageNormal[OCAM2_PIXELS_IMAGE_NORMAL];
        // Write a header with a "magic number" and a version
        short magic[1] = {static_cast<short>(10)};
        fwrite(magic,2,1,pFile);
        int data[3];
        data[0] = g_buffersize;
        data[1] = g_BP;
        data[2] = g_WP;
        fwrite(data,4,3,pFile);
        for(int k = 0; k < g_buffersize; k++){
            for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
                imageNormal[i] = g_surfacebuffer_short[k][i];
            }
            fwrite(imageNormal,2, OCAM2_PIXELS_IMAGE_NORMAL, pFile);
            ui->progressBar->setValue(k);
        }
    }else{
        // Allocate buffer for short
        // Write a header with a "magic number" and a version
        short magic[1] = {static_cast<short>(15)};
        fwrite(magic,2,1,pFile);
        int data[1];
        data[0] = g_buffersize;
        fwrite(data,4,1,pFile);
        for(int k = 0; k < g_buffersize; k++){
            for(int i = 0; i < 8448*2055; i++){
                g_save4kbuffer[i] = g_surfacebuffer_short[k][i];
            }
            fwrite(g_save4kbuffer,2, 8448*2055, pFile);
            ui->progressBar->setValue(k);
        }
    }
    fclose(pFile);
    qDebug() << "Time used: " << g_qtimeObj->elapsed()/1000.0 << "s";
}

// Load Raw Buffer
void MainWindow::on_BufferLoad_PB_clicked()
{
    qDebug() << "Load image buffer";
    int loadframes = 0;
    QString fileName = QFileDialog::getOpenFileName(this,"Open file",g_data_path+"/data","Files (*.dat)");
    if(fileName != ""){
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)){
            QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
        }else{
            g_qtimeObj->start();
            QDataStream in(&file);
            in.setByteOrder(QDataStream::LittleEndian);
            // Read and check the header
            qint16 magic;
            in >> magic;
            qDebug() << magic;
            in.setVersion(QDataStream::Qt_5_11);
            qint32 frames;
            qint32 data;
            qint16 val;
            if(magic != 10 && magic != 15){
                QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
            }
            else if(magic == 10){
                // OCam II
                in >> frames;
                loadframes = frames;
                g_BigImageBufferIndex = frames;
                ui->progressBar->setMaximum(loadframes-1);
                in >> data;
                g_BP = data;
                in >> data;
                g_WP = data;
                for(int j = 0; j < loadframes; j++){
                    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL;i++){
                        in >> g_BigImageBuffer[j][i];
                    }
                    ui->progressBar->setValue(j);
                    ui->ImageName_Label->setText(QString::number(j+1) + " / " +QString::number(loadframes));
                }
            }
            else if(magic == 15){
                // Simulateur 4K
                in >> frames;
                loadframes = frames;
                g_BigImageBufferIndex = frames;
                ui->progressBar->setMaximum(loadframes-1);
                for(int j = 0; j < loadframes; j++){
                    for(int i = 0; i < 8448*2055;i++){
                        in >> g_BigImageBuffer[j][i];
                    }
                    ui->progressBar->setValue(j);
                    ui->ImageName_Label->setText(QString::number(j+1) + " / " +QString::number(loadframes));
                }
            }
        }
        file.close();
    }
    g_load_correction = false;
    qDebug() << fileName << "loaded";
    qDebug() << "Time used: " << g_qtimeObj->elapsed()/1000.0 << "s";
    ChangeSection();
}

// Display Raw Buffer
void MainWindow::on_BufferDisplay_PB_clicked()
{
    g_qtimeObj->start();
    ui->progressBar->setMaximum(g_BigImageBufferIndex);
    if(!g_test4K){
        for(int i = 0; i < g_BigImageBufferIndex;i++){
            if(!g_load_correction)
                pixel_correction(g_BigImageBuffer[i],g_BigImageBuffer[i]);
            display(g_BigImageBuffer[i]);
            ui->progressBar->setValue(i+1);
            ui->ImageName_Label->setText(QString::number(i+1) + " / " +QString::number(g_BigImageBufferIndex));
            QTest::qWait(0);
        }
        g_load_correction = true;
    }else{
        vector<unsigned short> img_disp_fullsize(8448*2055,0);
        for(int i = 0; i < g_BigImageBufferIndex;i++){
            for(int j = 0; j < 8448*2055; j++){
                img_disp_fullsize[j] = g_BigImageBuffer[i][j] & 0xff;
            }
            if(g_sampling4k){
                g_img4K_disp = Sampling4k(img_disp_fullsize);
            }
            if(g_megapixel4k){
                g_img4K_disp = MegaPixel4k(img_disp_fullsize);
            }
            display_4K(g_img4K_disp);
            ui->progressBar->setValue(i+1);
            ui->ImageName_Label->setText(QString::number(i+1) + " / " +QString::number(g_BigImageBufferIndex));
            QTest::qWait(0);
        }
    }
    qDebug() << "Time used: " << g_qtimeObj->elapsed()/1000.0 << "s";
    ChangeSection();
}

// Thresh Save Button
void MainWindow::on_ThreshSave_PB_clicked()
{
    unsigned long g_mask_pixel = static_cast<unsigned long>(g_mask_x*g_mask_y);
    unsigned long times = static_cast<unsigned long>(OCAM2_PIXELS_IMAGE_NORMAL/(g_mask_pixel));
    ui->spinBox->setMaximum(static_cast<int>(times)-1);
    // Allocate memory for a 12*3600 matrix
    short **img_mask = new short*[times];
    for(unsigned long i = 0; i < times; i++){
        img_mask[i] = new short[g_mask_pixel];
    }
    // Vector of median(mean) value for each subimage
    float maskMED[times];
    int offset_x = 0;
    int offset_y = 0;
    short tmp[g_mask_pixel];
    // Save the pixel value to the 12*3600 matrix
    for(unsigned long j = 0; j < times; j++){
        int i = 0;
        offset_x = static_cast<int>(j) % (g_imgsize / g_mask_x) * g_mask_x;
        offset_y = g_mask_y * (static_cast<int>(j) / (g_imgsize / g_mask_y));
        for(int y = 0; y < g_mask_y; y++){
            for(int x = 0; x < g_mask_x; x++){
                img_mask[j][i] = g_imageNormal8bits[(offset_y * g_imgsize + offset_x) + x];    // 0-255
                i++;
            }
            offset_y++;
        }
        // Calculate subimage's median(mean)
        maskMED[j] = g_maths.Mean(short2float(img_mask[static_cast<int>(j)],g_mask_pixel));
    }

    int savemode[times];

    // Global median(mean) for the whole image
    float globalMED = g_maths.Mean(short2float(g_imageNormal8bits,OCAM2_PIXELS_IMAGE_NORMAL));

    // Chosse save mode, save the image or save only the address?
    for(unsigned long i = 0; i < times; i++){
        // Default value is 0, save image.
        savemode[i] = 0;
        // Save adress if subimage's median < global image's median(mean).
        if(maskMED[i] < globalMED){
            savemode[i] = 1;
        }
        cout << dec << i << "    " << maskMED[i] << "    ";
        if(savemode[i] == 0){
            cout << "image" << endl;
        }else{
            cout << "address" << endl;
        }
    }
    cout << globalMED << endl;
    ChangeSection();
    // Threshold sub image ( pixel's value < median(mean) => 0 )
    // Count the number of pixel which will be set to 0 in one subimage.
    int pixel_count[times];
    int rest_pixel[times];
    for(unsigned long i = 0; i < times; i++){
        // Initialize pixel count
        pixel_count[i] = 0;
        rest_pixel[i] = 0;
        // For each subimage, if the save mode is address, we count the number of pixel which is < subimage's median(mean).
        if(savemode[i] == 1){
            for(unsigned long j = 0; j < g_mask_pixel; j++){
                if(img_mask[i][j] < maskMED[i]){
                    pixel_count[i]++;
                }
                // For display
                if(static_cast<int>(i) == ui->spinBox->value()){
                    tmp[j] = img_mask[i][j];            // 0-255
                }
            }
            // If there are still more than half pixel, we save the image.
            if(g_mask_pixel - static_cast<unsigned long>(pixel_count[i]) > g_mask_pixel/2){
                savemode[i] = 0;
                cout << i << " addresse ==> image " << g_mask_pixel - static_cast<unsigned long>(pixel_count[i]) << endl;
            }
            // Set pixel value to 0 after memory check.
            else{
                rest_pixel[i] = static_cast<int>(g_mask_pixel) - pixel_count[i];
                for(unsigned long j = 0; j < g_mask_pixel; j++){
                    if(img_mask[i][j] < maskMED[i]){
                        img_mask[i][j] = 0;
                    }
                }
            }
        }
    }
    // Save the image
    ChangeSection();
    ChangeSection();
    QString filename = "/acqui/data/threshsave_"+QString::number(g_num-1) + ".dat";
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        QDataStream out(&file);
        // Magic number
        out << static_cast<qint16>(0xee);
        out.setVersion(QDataStream::Qt_5_11);
        out << static_cast<qint32>(g_num);
        out << static_cast<qint16>(g_BP);
        out << static_cast<qint16>(g_WP);
        for(unsigned long i = 0; i < times; i++){
            //            ChangeSection();
            cout << i << "th subimage ===> ";
            if(savemode[i] == 0){
                cout << "image" << endl;
            }else{
                cout << "address" << endl;
            }
            //            ChangeSection();
            // Save image
            if(savemode[i] == 0){
                out << static_cast<QString>("img");
                for(unsigned long j = 0; j < g_mask_pixel; j++){
                    out << static_cast<quint16>(img_mask[i][j]);
                    //                    cout << static_cast<int>(i)*g_mask_x*g_mask_y+static_cast<int>(j) << "    " << img_mask[i][j] << endl;
                }
            }
            // Save address
            else{
                out << static_cast<QString>("add");
                out << static_cast<qint16>(rest_pixel[i]);
                // Save first the position than the value
                for(unsigned long j = 0; j < g_mask_pixel; j++){
                    if(img_mask[i][j] != 0){
                        out << static_cast<quint16>(j);
                        out << static_cast<qint16>(img_mask[i][j]);
                        //                        cout << i*g_mask_x*g_mask_y+j << "    " << img_mask[i][j] << endl;
                    }else{
                        //                        cout << i*g_mask_x*g_mask_y+j << "    " << img_mask[i][j] << " --------------- " << endl;
                    }

                }
            }
        }
    }
    file.close();
    qDebug() << filename << "saved.";
    ChangeSection();
    for(unsigned long i = 0; i < times; i++){
        if(savemode[i] != 0)
            cout << dec << i << "th subimage ===> " << pixel_count[i] << " pixels" << endl;
    }
    // Free memory
    for(unsigned long i = 0; i < times; i++){
        delete[] img_mask[i];
    }
    delete[] img_mask;
    // Display
    *g_image = QImage(g_mask_x,g_mask_y,QImage::Format_RGB888);
    for (unsigned long pos=0,i=0; i < g_mask_pixel; pos+=3,i++)
    {
        unsigned char PixRed=0, PixGreen=0, PixBlue=0;
        short RawPixel= tmp[i];
        if (RawPixel >= 0xFF) //if pixel is above White point
            PixRed= PixGreen= PixBlue= 0xFF;//8 bit value is 255
        else if (RawPixel <= 0)//if below Black
            PixRed= PixGreen= PixBlue = 0;//value is 0
        else{
            PixRed= PixGreen= PixBlue= static_cast<unsigned char>(static_cast<int>(RawPixel));
        }
        g_image->bits()[pos] = PixRed;
        g_image->bits()[pos+1] = PixGreen;
        g_image->bits()[pos+2] = PixBlue;
    }
    g_scene2->clear();
    g_scene2->addPixmap(QPixmap::fromImage(*g_image));
    ui->graphicsView_2->setScene(g_scene2);
    ui->graphicsView_2->show();

}

// Thresh Load Button
void MainWindow::on_ThreshLoad_PB_clicked()
{
    unsigned long times = static_cast<unsigned long>(OCAM2_PIXELS_IMAGE_NORMAL/(g_mask_x*g_mask_y));
    ui->spinBox->setMaximum(static_cast<int>(times)-1);
    // Allocate memory for loaded image
    short imageThreshLoad[OCAM2_PIXELS_IMAGE_NORMAL];
    short **img_mask = new short*[times];
    for(unsigned long i = 0; i < times; i++){
        img_mask[i] = new short[static_cast<unsigned long>(g_mask_pixel)];
    }
    // reset load image memory to 0;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        imageThreshLoad[i] = 0;
    }
    for(unsigned long i = 0; i < times; i++){
        for(int j = 0; j < g_mask_pixel; j++){
            img_mask[i][j] = 0;
        }
    }
    int loadnum = 0;
    // Load image
    QString fileName = QFileDialog::getOpenFileName(this,"Open file",g_data_path,"Files (*.dat)");    if(fileName != ""){
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)){
            QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
        }else{
            QDataStream in(&file);
            // Read and check the header
            qint16 magic;
            in >> magic;
            if(magic != 0xee)
                QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
            else{
                in.setVersion(QDataStream::Qt_5_11);
                qint32 headnum;
                in >> headnum;
                loadnum = headnum;
                qint16 data;
                in >> data;
                g_BP = data;
                in >> data;
                g_WP = data;
                QString savemode;
                quint16 udata;
                for(unsigned long i = 0; i < times; i++){
                    in >> savemode;
                    QString mode = savemode;
                    // Save image
                    if(mode == "img"){
                        for(int j = 0; j < g_mask_pixel; j++){
                            in >> data;
                            img_mask[i][j] = data;
                            //                            cout << dec << i*g_mask_x*g_mask_y+j << "    " << img_mask[i][j] << endl;
                        }
                    }
                    // Save address
                    else if(mode == "add"){
                        in >> udata;
                        unsigned short pixel_nb = udata;
                        for(int j = 0; j < pixel_nb; j++){
                            in >> udata;
                            in >> data;
                            img_mask[i][udata] = data;
                            //                            cout << dec << i*g_mask_x*g_mask_y+udata << "  address  " << img_mask[i][udata] << endl;
                        }
                    }
                }
            }
        }
        file.close();
    }
    // Assembly subimage to initial image
    int offset_x = 0;
    int offset_y = 0;
    for(unsigned long j = 0; j < times; j++){
        int i = 0;
        offset_x = static_cast<int>(j) % (g_imgsize / g_mask_x) * g_mask_x;
        offset_y = g_mask_y * (static_cast<int>(j) / (g_imgsize / g_mask_y));
        for(int y = 0; y < g_mask_y; y++){
            for(int x = 0; x < g_mask_x; x++){
                imageThreshLoad[(offset_y * g_imgsize + offset_x) + x] = img_mask[j][i];
                i++;
            }
            offset_y++;
        }
    }
    // Free memory
    for(unsigned long i = 0; i < times; i++){
        delete[] img_mask[i];
    }
    delete[] img_mask;
    // Display
    *g_image = QImage(240,240,QImage::Format_RGB888);
    for (int pos=0,i=0; i < OCAM2_PIXELS_IMAGE_NORMAL; pos+=3,i++)
    {
        unsigned char PixRed=0, PixGreen=0, PixBlue=0;
        short RawPixel= imageThreshLoad[i];
        if (RawPixel >= 0xFF) //if pixel is above White point
            PixRed= PixGreen= PixBlue= 0xFF;//8 bit value is 255
        else if (RawPixel <= 0)//if below Black
            PixRed= PixGreen= PixBlue = 0;//value is 0
        else{
            PixRed= PixGreen= PixBlue= static_cast<unsigned char>(static_cast<int>(RawPixel));
        }
        g_image->bits()[pos] = PixRed;
        g_image->bits()[pos+1] = PixGreen;
        g_image->bits()[pos+2] = PixBlue;
    }
    g_scene->clear();
    g_scene->addPixmap(QPixmap::fromImage(*g_image));
    ui->graphicsView->setScene(g_scene);
    ui->graphicsView->show();
    qDebug() << fileName << "loaded";
}

// Calculate register's statistics
void MainWindow::on_MaskStatistics_PB_clicked()
{
    // Initialization
    unsigned long frames = 100;
    ui->Mask_processBar->setMaximum(frames-1);
    //    ui->graphicsView_2->resize(g_mask_x+5,g_mask_y+5);
    int nb_register = OCAM2_PIXELS_IMAGE_NORMAL/(g_mask_pixel);
    ui->spinBox->setMaximum(nb_register-1);
    ui->MaskStatistics_PB->setFocus();
    if(ui->MaskStatistics_PB->text() == "Restart"){
        ui->statistics_label->setText("1");
        ui->MaskStatistics_PB->setText("Mask Statistics");
    }
    // Allocate memory for vectors
    vector<vector< vector <float>>> img_mask;
    vector<vector< vector <float>>> img_dark_mask;
    img_mask.resize(nb_register);
    for(int i = 0; i < nb_register; i++){
        img_mask[i].resize(g_mask_pixel);
        for(int j = 0; j < g_mask_pixel; j++){
            img_mask[i][j].resize(frames);
        }
    }
    img_dark_mask.resize(nb_register);
    for(int i = 0; i < nb_register; i++){
        img_dark_mask[i].resize(g_mask_pixel);
        for(int j = 0; j < g_mask_pixel; j++){
            img_dark_mask[i][j].resize(frames);
        }
    }
    int offset_x = 0;
    int offset_y = 0;
    // Initiate statistic vector
    //    vector<vector <float>> median_list(nb_register, vector<float>(g_mask_pixel,0));
    //    vector<vector <float>> mean_list(nb_register, vector<float>(g_mask_pixel,0));
    //    vector<vector <float>> var_list(nb_register, vector<float>(g_mask_pixel,0));
    //    vector<vector <float>> median_ID_list(nb_register, vector<float>(g_mask_pixel,0));
    //    vector<vector <float>> mean_ID_list(nb_register, vector<float>(g_mask_pixel,0));
    //    vector<vector <float>> var_ID_list(nb_register, vector<float>(g_mask_pixel,0));
    float median[nb_register], mean[nb_register], var[nb_register];
    float median_ID[nb_register], mean_ID[nb_register], var_ID[nb_register];
    vector<float> global_I_var_list(frames);
    vector<float> global_I_D_mean_list(frames);
    vector<float> global_I_D_var_list(frames);
    ChangeSection();
    // Grab images
    MCSTATUS status = 0;
    // Start Acquisitions for this channel.
    status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    if(status != MC_OK){
        test_error();
    }
    // Allocate memory for image and image-dark
    vector<vector <float>> images(OCAM2_PIXELS_IMAGE_NORMAL, vector<float>(frames,0));
    vector<vector <float>> images_dark(OCAM2_PIXELS_IMAGE_NORMAL, vector<float>(frames,0));
    short **images_normal = new short*[frames];
    for(unsigned long i = 0; i < frames; i++){
        images_normal[i] = new short[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    // Grabbed images stored in "images" (240*240)
    for(unsigned long i = 0; i < frames; i++){
        status = McSetParamInt(g_hChannel, MC_ForceTrig, MC_ForceTrig_TRIG);
        if(status != MC_OK){
            test_error();
        }
        ocam2_descramble(g_id, &g_number, images_normal[i], g_imageRawNormal);
        // Assign value for image
        for(int k = 0; k < OCAM2_PIXELS_IMAGE_NORMAL; k++){
            images[k][i] = images_normal[i][k] & 0x3fff;
        }
        // Assign value for image-dark
        for(int k = 0; k < OCAM2_PIXELS_IMAGE_NORMAL; k++){
            images_dark[k][i] = images[k][i] - g_imageNormalDark[k];
            if(images_dark[k][i] < 0){
                images_dark[k][i] = 0;
            }
        }
    }
    // Divide 240*240 image to 8 subimage for each register
    for(unsigned long k = 0; k < frames; k++){
        for(int j = 0; j < nb_register; j++){
            int i = 0;
            offset_x = j % (g_imgsize / g_mask_x) * g_mask_x;
            offset_y = g_mask_y * (int(j / (g_imgsize / g_mask_x)));
            for(int y = 0; y < g_mask_y; y++){
                for(int x = 0; x < g_mask_x; x++){
                    img_mask[j][y*g_mask_x+x][k] = images[(offset_y * g_imgsize + offset_x) + x][k];
                    img_dark_mask[j][y*g_mask_x+x][k] = images_dark[(offset_y * g_imgsize + offset_x) + x][k];
                    i++;
                }
                offset_y++;
            }
        }
    }
    // Calculate mask image statistics
    vector<vector <float>> img_reg_mean(nb_register, vector<float>(g_mask_pixel,0));
    vector<vector <float>> img_reg_median(nb_register, vector<float>(g_mask_pixel,0));
    vector<vector <float>> img_reg_var(nb_register, vector<float>(g_mask_pixel,0));
    vector<vector <float>> img_dark_reg_mean(nb_register, vector<float>(g_mask_pixel,0));
    vector<vector <float>> img_dark_reg_median(nb_register, vector<float>(g_mask_pixel,0));
    vector<vector <float>> img_dark_reg_var(nb_register, vector<float>(g_mask_pixel,0));

    for(int i = 0; i < nb_register; i++){
        for(int j = 0; j < g_mask_pixel; j++){
            img_reg_mean[i][j] = g_maths.Mean(img_mask[i][j]);
            img_reg_median[i][j] = g_maths.Median(img_mask[i][j]);
            img_reg_var[i][j] = g_maths.Variance(img_mask[i][j]);
            img_dark_reg_mean[i][j] = g_maths.Mean(img_dark_mask[i][j]);
            img_dark_reg_median[i][j] = g_maths.Median(img_dark_mask[i][j]);
            img_dark_reg_var[i][j] = g_maths.Variance(img_dark_mask[i][j]);
        }
    }

    for(int i = 0; i < nb_register; i++){
        mean[i] = g_maths.Mean(img_reg_mean[i]);
        median[i] = g_maths.Mean(img_reg_median[i]);
        var[i] = g_maths.Mean(img_reg_var[i]);
        mean_ID[i] = g_maths.Mean(img_dark_reg_mean[i]);
        median_ID[i] = g_maths.Mean(img_dark_reg_median[i]);
        var_ID[i] = g_maths.Mean(img_dark_reg_var[i]);
    }

    // Calculate global image statistics
    for(int k = 0; k < frames; k++){
        global_I_D_mean_list[k] = g_maths.Mean(images_dark[k]);
        global_I_var_list[k] = g_maths.Variance(images[k]);
        global_I_D_var_list[k] = g_maths.Variance(images_dark[k]);
        ui->Mask_processBar->setValue(k);
    }

    //            mean_list[j][k] = g_maths.Mean(short2float(img_mask,g_mask_pixel));
    //            median_list[j][k] = g_maths.Median(short2float(img_mask,g_mask_pixel));
    //            var_list[j][k] = g_maths.Variance(short2float(img_mask,g_mask_pixel));
    //            median_ID_list[j][k] = g_maths.Median(short2float(img_dark_mask,g_mask_pixel));
    //            mean_ID_list[j][k] = g_maths.Mean(short2float(img_dark_mask,g_mask_pixel));
    //            var_ID_list[j][k] = g_maths.Variance(short2float(img_dark_mask,g_mask_pixel));
    //            // Calculate global image statistics
    //            global_I_D_mean_list[k] = g_maths.Mean(images_dark[k]);
    //            global_I_var_list[k] = g_maths.Variance(images[k]);
    //            global_I_D_var_list[k] = g_maths.Variance(images_dark[k]);
    //        }
    //        ui->Mask_processBar->setValue(k);
    //    }
    //    for(int i = 0; i < nb_register; i++)
    //    {
    //        mean[i] = g_maths.Median(mean_list[i]);
    //        median[i] = g_maths.Median(median_list[i]);
    //        var[i] = g_maths.Median(var_list[i]);
    //        mean_ID[i] = g_maths.Median(mean_ID_list[i]);
    //        median_ID[i] = g_maths.Median(median_ID_list[i]);
    //        var_ID[i] = g_maths.Median(var_ID_list[i]);
    //    }

    // Fill table
    int stats_num = ui->statistics_label->text().toInt();
    QTableWidgetItem *item = nullptr;
    for(int i = 0; i < 8; i++){
        item = new QTableWidgetItem(QString::number(mean[i]));
        ui->image_table->setItem(i,0,item);
        item = new QTableWidgetItem(QString::number(median[i]));
        ui->image_table->setItem(i,1,item);
        item = new QTableWidgetItem(QString::number(var[i]));
        ui->image_table->setItem(i,2,item);
        item = new QTableWidgetItem(QString::number(mean_ID[i]));
        item = new QTableWidgetItem(QString::number(median_ID[i]));
        item = new QTableWidgetItem(QString::number(var_ID[i]));
        g_plot_I_Var[i][stats_num-1] = var[i];
        g_plot_I_D_Mean[i][stats_num-1] = mean_ID[i];
        g_plot_I_D_Var[i][stats_num-1] = var_ID[i];
    }
    // Get global statistics
    g_plot_g_I_D_Mean[stats_num-1] = g_maths.Mean(global_I_D_mean_list);
    g_plot_g_I_Var[stats_num-1] = g_maths.Mean(global_I_var_list);
    g_plot_g_I_D_Var[stats_num-1] = g_maths.Mean(global_I_D_var_list);
    cout << dec << stats_num << "th acquisition done, " << stats_num << " / " << g_statistics_try << " complet." << endl;
    if(stats_num < g_statistics_try){
        ui->statistics_label->setText(QString::number(stats_num+1));
    }else{
        ui->statistics_label->setText("End");
        ui->MaskStatistics_PB->setText("Restart");
    }
    /*    // Convert to 8-bits for display
//    for(int i = 0; i < g_mask_pixel; i++){
//        // Inverse 2 bit
//        int temp = tmp[i];
//        temp = ((temp-static_cast<int>(g_BP))*255);
//        temp = temp/(static_cast<int>(g_WP)-static_cast<int>(g_BP));
//        if(temp < 0)
//            tmp[i] = 0;
//        else if(temp > 255)
//            tmp[i] = 255;
//        else
//            tmp[i] = static_cast<short>(temp);
//    }
//    // Display
//    *g_image = QImage(g_mask_x,g_mask_y,QImage::Format_RGB888);
//    for (int pos=0,i=0; i < g_mask_pixel; pos+=3,i++)
//    {
//        unsigned char PixRed=0, PixGreen=0, PixBlue=0;
//        short RawPixel= tmp[i];
//        if (RawPixel >= 0xFF) //if pixel is above White point
//            PixRed= PixGreen= PixBlue= 0xFF;//8 bit value is 255
//        else if (RawPixel <= 0)//if below Black
//            PixRed= PixGreen= PixBlue = 0;//value is 0
//        else{
//            PixRed= PixGreen= PixBlue= static_cast<unsigned char>(static_cast<int>(RawPixel));
//        }
//        g_image->bits()[pos] = PixRed;
//        g_image->bits()[pos+1] = PixGreen;
//        g_image->bits()[pos+2] = PixBlue;
//    }
//    g_scene2->clear();
//    g_scene2->addPixmap(QPixmap::fromImage(*g_image));
//    ui->graphicsView_2->setScene(g_scene2);
//    ui->graphicsView_2->show();
*/
}

// Statistics button
void MainWindow::on_Statistics_PB_clicked()
{
    // Global statistics
    g_meanvalue = g_maths.Mean(short2float(g_imageNormal,OCAM2_PIXELS_IMAGE_NORMAL));
    g_median = g_maths.Median(short2float(g_imageNormal,OCAM2_PIXELS_IMAGE_NORMAL));
    g_variance = g_maths.Variance(short2float(g_imageNormal,OCAM2_PIXELS_IMAGE_NORMAL));
    g_SD = g_maths.StandardDeviation(short2float(g_imageNormal,OCAM2_PIXELS_IMAGE_NORMAL));
    g_SNR = g_meanvalue / g_SD;
    ui->Mean_Label->setText("Mean: " + QString::number(static_cast<double>(g_meanvalue)));
    ui->Median_Label->setText("Median: " + QString::number(g_median));
    ui->Variance_Label->setText("Variance: " + QString::number(static_cast<double>(g_variance)));
    ui->StandardDeviation_Label->setText("Ecart-type: " + QString::number(static_cast<double>(g_SD)));
    ui->SNR_Label->setText("SNR: " + QString::number(static_cast<double>(g_SNR)));
    //    ui->Statistics_PB->setEnabled(false);

    // Register statistics
    int nb_register = OCAM2_PIXELS_IMAGE_NORMAL/(g_mask_pixel);
    short img_mask[g_mask_pixel];
    int offset_x = 0;
    int offset_y = 0;
    // Initiate statistic vector
    float mean[nb_register], var[nb_register], sd[nb_register];
    // Divide 240*240 image to 8 subimage for each register
    for(int j = 0; j < nb_register; j++){
        int i = 0;
        offset_x = j % (g_imgsize / g_mask_x) * g_mask_x;
        offset_y = g_mask_y * (int(j / (g_imgsize / g_mask_x)));
        for(int y = 0; y < g_mask_y; y++){
            for(int x = 0; x < g_mask_x; x++){
                img_mask[i] = g_imageNormal[(offset_y * g_imgsize + offset_x) + x];
                i++;
            }
            offset_y++;
        }
        // Calculate mask image statistics
        mean[j] = g_maths.Mean(short2float(img_mask,g_mask_pixel));
        var[j] = g_maths.Variance(short2float(img_mask,g_mask_pixel));
        sd[j] = g_maths.StandardDeviation(short2float(img_mask,g_mask_pixel));
    }
    // Fill table
    QTableWidgetItem *item = nullptr;
    for(int i = 0; i < 8; i++){
        item = new QTableWidgetItem(QString::number(mean[i]));
        ui->image_table->setItem(i,0,item);
        item = new QTableWidgetItem(QString::number(var[i]));
        ui->image_table->setItem(i,1,item);
        item = new QTableWidgetItem(QString::number(sd[i]));
        ui->image_table->setItem(i,2,item);
    }
}

// Display Histogram
void MainWindow::on_Histo_PB_clicked()
{
    // Open dialog
    Dialog_Histogram histo(this);

    // Calculate histogram
    histo.max = 0;
    histo.maxpos = 0;
    for(int i = 0; i < 256; i++){
        histo.hist[i] = 0;
    }

    for(int i = 0; i < 256; i++){
        for(int j = 0; j < OCAM2_PIXELS_IMAGE_NORMAL; j++){
            if(g_imageNormal8bits[j]==i)
                histo.hist[i]++;
        }
        if(histo.max < histo.hist[i]){
            histo.max = histo.hist[i];
            histo.maxpos = i;
        }
    }
    histo.exec();
}

// Save PNG Button
void MainWindow::on_PNG_PB_clicked()
{
    // Create 16-bits MAT
    Mat img(4096,4096,CV_16UC1,Scalar(0));
    for (int j = 0; j < img.rows; j++){
        for(int i = 0; i < img.cols; i++){
            if(g_img_4K[j*4096+i] > 16383){
                g_img_4K[j*4096+i] = 16383;
            }
            if(g_img_4K[j*4096+i] < 0){
                g_img_4K[j*4096+i] = 0;
            }
            img.at<ushort>(j,i) = static_cast<unsigned short>(g_img_4K[j*4096+i]);
        }
    }
    //    imshow(to_string(g_num), img);
    string filepath = "/acqui/png/" + to_string(g_num-1) + ".png";
    imwrite(filepath,img);
    cout << filepath << " saved." << endl;
}

// Get Temperature
void MainWindow::on_Temp_PB_clicked()
{
    SerialCommand("temp",0);
}

// Set the temperature to -45°C
void MainWindow::on_Temp_n45_PB_clicked()
{
    SerialCommand("temp -45",0);
}

// Set the temperature to 30°C
void MainWindow::on_Temp_p30_PB_clicked()
{
    SerialCommand("temp 30", 0);
}

// Set the cooling off
void MainWindow::on_Temp_off_PB_clicked()
{
    SerialCommand("temp off", 0);
}

// Reset overillumination protection
void MainWindow::on_Overillumination_Reset_PB_clicked()
{
    SerialCommand("protection reset",0);
}

// Clear Command History
void MainWindow::on_ClearHistory_PB_clicked()
{
    ui->Console_Output_TextEdit->clear();
}

// Acquire Cycle Button
void MainWindow::on_AcquireCycle_PB_clicked()
{
    // Initialize cycle buffer
    for(int j = 0; j < g_nb_cycle; j++){
        g_image_cycle[j] = new short[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        g_image_cycle_sum[i] = 0;
        for(int j = 0; j < g_nb_cycle; j++){
            g_image_cycle[j][i] = 0;
        }
    }
    // Acquire Cycle
    for(int j = 0; j < g_nb_cycle; j++){
        AcquireImages();
        // Assign value to cycle buffer
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            g_image_cycle[j][i] = g_imageNormal8bits[i];
        }
        usleep(500);
    }
    // Calculate cycle sum
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        for(int j = 0; j < g_nb_cycle; j++){
            g_image_cycle_sum[i] += g_image_cycle[j][i];
        }
    }
    cout << "Cycle buffer acquired." << endl;

    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        g_image_cycle_sum[i] /= g_nb_cycle;
    }
    //    // σ²Read
    //    float sigma2Read = 0;
    //    for(int j = 0; j < g_nb_cycle - 1; j++){
    //        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
    //            sigma2Read += pow(static_cast<float>(g_image_cycle[j][i] - g_image_cycle[j+1][i]), 2);
    //        }
    //    }
    //    sigma2Read = sigma2Read /( 2 * (g_nb_cycle - 1) * OCAM2_PIXELS_IMAGE_NORMAL);
    //    cout << dec << "σ²Read = " << sigma2Read << endl;
    //    float sigmaR = 5*sqrt(sigma2Read);
    //    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
    //        g_image_cycle_sum[i] = (g_image_cycle_sum[i] <  sigmaR) ? 0 : g_image_cycle_sum[i];
    //    }
    // Display mean sum
    display(g_image_cycle_sum);
}

// Save Statistics Button
void MainWindow::on_PlotStatistics_PB_clicked()
{
    // Open dialog
    Dialog_stats stats(this);
    stats.exec();
}

// Save Button
void MainWindow::on_Save_PB_clicked()
{
    // Suppressing read noise
    float thresh = 5*sqrt(g_Sigma2Read);
    vector<float> img_threshed(OCAM2_PIXELS_IMAGE_NORMAL,0);
    cout << dec << short(thresh) << endl;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        //        img_threshed[i] = (g_imageNormal[i] < 5*sigmaRead) ? 0 : g_imageNormal[i];
        if(g_imageNormal[i] < short(thresh)){
            img_threshed[i] = 0;
            cout << dec << "Threshed, from " << g_imageNormal[i] << " to 0, thresh = "  << thresh << endl;
        }
        else if(g_imageNormal[i] == short(thresh)){
            img_threshed[i] = 1;
        }
        else{
            img_threshed[i] = g_imageNormal[i];
            cout << dec << "Multi-events, " << g_imageNormal[i] << endl;
        }
    }
    // Analyzing image
    unsigned int address_count = 0, mask_count = 0, zero_count = 0;
    vector<unsigned int> address, mask;
    vector<unsigned short> data;
    for(unsigned int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        if(img_threshed[i] == 1){
            address.push_back(i);
        }
        if(img_threshed[i] > 1){
            mask.push_back(i);
            data.push_back(img_threshed[i]);
        }
    }
    address_count = address.size();
    mask_count = mask.size();
    zero_count = OCAM2_PIXELS_IMAGE_NORMAL - address_count - mask_count;
    cout << dec << "zero pixel count: " << zero_count << " " << static_cast<double>(zero_count)/static_cast<double>(OCAM2_PIXELS_IMAGE_NORMAL)*100 << "%" << endl;
    cout << dec <<  "address count: " << address_count << " " << static_cast<double>(address_count)/static_cast<double>(OCAM2_PIXELS_IMAGE_NORMAL)*100 << "%" << endl;
    cout << dec << "mask_count: " << mask_count << " " << static_cast<double>(mask_count)/static_cast<double>(OCAM2_PIXELS_IMAGE_NORMAL)*100 << "%"  << endl;
    // Saving
    QString filename = "/acqui/data/dat_"+QString::number(g_num-1) + ".dat";
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        QDataStream out(&file);
        out << static_cast<qint16>(0xff);
        out.setVersion(QDataStream::Qt_5_11);
        out << static_cast<quint32>(g_num);
        out << static_cast<quint32>(address_count);
        out << static_cast<quint32>(mask_count);
        for(unsigned int i = 0; i < address_count; i++){
            out << static_cast<quint32>(address[i]);
        }
        for(unsigned int i = 0; i < mask_count; i++){
            out << static_cast<quint32>(mask[i]);
            out << static_cast<quint16>(data[i]);
        }
    }
    file.close();
    qDebug() << filename << " saved." << endl;
}

// Load Button
void MainWindow::on_Load_PB_clicked()
{
    // Reset g_imageNormal to 0
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        g_imageNormal[i] = 0;
    }
    ui->Statistics_PB->setEnabled(true);
    // Initialization
    unsigned int loadnum = 0, address_count = 0, mask_count = 0, zero_count = 0;
    vector<unsigned int> address_vec, mask_vec;
    // Loading
    QString fileName = QFileDialog::getOpenFileName(this,"Open file",g_data_path,"Files (*.dat)");
    if(fileName != ""){
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)){
            QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
        }else{
            QDataStream in(&file);
            // Read and check the header
            qint16 magic;
            in >> magic;
            if(magic != 0xff)
                QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
            else{
                in.setVersion(QDataStream::Qt_5_11);
                quint32 address;
                quint16 data;
                in >> loadnum;
                in >> address_count;
                in >> mask_count;
                zero_count = OCAM2_PIXELS_IMAGE_NORMAL - address_count - mask_count;
                cout << dec << "zero pixel count: " << zero_count << " " << static_cast<double>(zero_count)/static_cast<double>(OCAM2_PIXELS_IMAGE_NORMAL)*100 << "%" << endl;
                cout << dec <<  "address count: " << address_count << " " << static_cast<double>(address_count)/static_cast<double>(OCAM2_PIXELS_IMAGE_NORMAL)*100 << "%" << endl;
                cout << dec << "mask_count: " << mask_count << " " << static_cast<double>(mask_count)/static_cast<double>(OCAM2_PIXELS_IMAGE_NORMAL)*100 << "%"  << endl;
                for(unsigned int i = 0; i < address_count; i++){
                    in >> address;
                    g_imageNormal[address] = 1;
                    address_vec.push_back(address);
                }
                for(unsigned int i = 0; i < mask_count; i++){
                    in >> address;
                    in >> data;
                    g_imageNormal[address] = data;
                    mask_vec.push_back(address);
                }
            }
        }
        file.close();
    }
    pixel_correction(g_imageNormal,g_imageNormal8bits);
    display(g_imageNormal8bits);
    ui->ImageName_Label->setText(QString::number(loadnum-1) + ".dat");
    qDebug() << fileName << "loaded";
    // Display mask
    Mat img(240,240,CV_8UC3,Scalar(0));
    vector<Vec3b> colorTab;
    // Plot mask
    for(unsigned int i = 0; i < mask_count; i++){
        img.at<Vec3b>(mask_vec[i]/img.cols,mask_vec[i]%img.cols)[0] = 255;
        img.at<Vec3b>(mask_vec[i]/img.cols,mask_vec[i]%img.cols)[1] = 0;
        img.at<Vec3b>(mask_vec[i]/img.cols,mask_vec[i]%img.cols)[2] = 0;
    }
    // Plot 1
    for(unsigned int i = 0; i < address_count; i++){
        img.at<Vec3b>(address_vec[i]/img.cols,address_vec[i]%img.cols)[0] = 0;
        img.at<Vec3b>(address_vec[i]/img.cols,address_vec[i]%img.cols)[1] = 0;
        img.at<Vec3b>(address_vec[i]/img.cols,address_vec[i]%img.cols)[2] = 255;
    }
    imshow("Mask", img);
}

// Add Gaussian Noise
void MainWindow::on_AddNoise_PB_clicked()
{
    float mu = 29, sigma = 0.1;
    // Random device class instance, source of 'true' randomness for initializing random seed
    random_device rd{};
    // Mersenne twister PRNG, initialized with seed from previous random device instance
    mt19937_64 gen{rd()};
    // Instance of class std::normal_distribution with specific mean and stddev
    normal_distribution<float> gaussian_noise(mu,sigma);
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        g_imageNormal[i] += gaussian_noise(gen);
    }
    pixel_correction(g_imageNormal,g_imageNormal8bits);
    display(g_imageNormal8bits);
}

// Get Mask Button
void MainWindow::on_GetMask_PB_clicked()
{
    // Initialization
    cout << "Calculating Mask ... " << endl;
    g_surfacebuffercount = 0;
    g_callback_mode = 0;
    unsigned int frames = 2000;
    g_buffersize = frames;
    g_surfacebuffer = new unsigned char *[frames];
    for(int i = 0; i < frames; i++){
        g_surfacebuffer[i] = new unsigned char[g_sizeX*g_sizeY];
    }
    int gain = 29;
    float thresh = 5*sqrt(g_Sigma2Read);
    short **img = new short*[frames];
    for(unsigned int j = 0; j < frames; j++){
        img[j] = new short[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    unsigned int signal_count = 0;
    vector<vector <short>> mode(OCAM2_PIXELS_IMAGE_NORMAL, vector<short>(frames,0));
    // Grab 2000 images
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    while(g_surfacebuffercount < frames-1);
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    if(g_status != MC_OK){
        test_error();
    }
    for(int k = 0; k < frames; k++){
        // Merge two 8-bit char to a 16-bit short
        for(int j = 0;j<OCAM2_PIXELS_RAW_NORMAL;j++){
            g_imageRawNormal[j] = (static_cast<short>((g_surfacebuffer[k][j*2+1])<<8)) | (g_surfacebuffer[k][j*2]);
        }
        ocam2_descramble(g_id, &g_number, g_imageNormal, g_imageRawNormal);
        // Convert to 14 bits data
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            g_imageNormal[i] = g_imageNormal[i] & 0x3fff;
            img[k][i] = g_imageNormal[i];
        }
    }

    // Subtraction of dark image
    for(unsigned int j = 0; j < frames; j++){
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            img[j][i] = img[j][i] - g_imageNormalDark[i];
            if(img[j][i] < 0)
                img[j][i] = 0;
        }
    }

    // Threshold 5σRead
    for(unsigned int j = 0; j < frames; j++){
        for(unsigned int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            //            cout << j << "th frame, " << i << "th pixel, " << img[j][i] << "    " << thresh/gain << "    " << static_cast<double>(double((img[j][i])/50) - thresh / double(gain)) << endl;
            if(static_cast<double>(double((img[j][i])/g_gain_value) - thresh / double(gain)) < 1){
                mode[i][j] = 0;
            }else{
                mode[i][j] = 1;
            }
        }
    }

    // Calculate mask
    vector<unsigned int> sum_mode(OCAM2_PIXELS_IMAGE_NORMAL,0);
    vector<unsigned int> signal_mask;
    for(unsigned int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        for(unsigned int j = 0; j < frames; j++){
            sum_mode[i] += mode[i][j];
        }
        if(sum_mode[i] != 0)
            signal_mask.push_back(i);
    }
    signal_count = signal_mask.size();
    // Save mask
    QString filename = "/acqui/data/mask.dat";
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly)){
        QDataStream out(&file);
        out.setByteOrder(QDataStream::LittleEndian);
        out << static_cast<qint16>(0xff);
        out.setVersion(QDataStream::Qt_5_11);
        out << static_cast<qint16>(gain);
        out << static_cast<quint32>(signal_count);
        for(unsigned int i = 0; i < signal_count; i++){
            out << static_cast<quint32>(signal_mask[i]);
        }
    }
    file.close();
    qDebug() << filename << "saved." << endl;
    // Display mask
    Mat mask(g_imgsize,g_imgsize,CV_8UC3,Scalar(0));
    // Plot mask
    for(unsigned int i = 0; i < signal_count; i++){
        mask.at<Vec3b>(signal_mask[i]/mask.cols,signal_mask[i]%mask.cols)[0] = 0;
        mask.at<Vec3b>(signal_mask[i]/mask.cols,signal_mask[i]%mask.cols)[1] = 255;
        mask.at<Vec3b>(signal_mask[i]/mask.cols,signal_mask[i]%mask.cols)[2] = 0;
    }
    imshow("Mask", mask);
    cout << "Done ..." << endl;
}

// Acquire Set Button
void MainWindow::on_AcquireSet_PB_clicked()
{
    // Initialization
    cout << "Loading Mask File ..." << endl;
    g_buffersize = ui->Sequence_spinBox->value();
    unsigned int frames = g_buffersize;
    unsigned int signal_count = 0;
    vector<unsigned int> signal_mask;
    short **img = new short*[frames];
    for(unsigned int j = 0; j < frames; j++){
        img[j] = new short[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    ui->progressBar->setMaximum(frames);
    // Loading
    QString maskfilename = "/acqui/data/mask.dat";
    QFile maskfile(maskfilename);
    if(!maskfile.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
    }else{
        QDataStream in(&maskfile);
        in.setByteOrder(QDataStream::LittleEndian);
        // Read and check the header
        qint16 magic;
        in >> magic;
        if(magic != 0xff)
            QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
        else{
            in.setVersion(QDataStream::Qt_5_11);
            qint16 gain;
            in >> gain;
            in >> signal_count;
            signal_mask.resize(signal_count);
            for(int i = 0; i < signal_count;i++){
                in >> signal_mask[i];
            }
        }
    }
    maskfile.close();
    qDebug() << maskfilename << "loaded.";
    // Display mask
    Mat mask(g_imgsize,g_imgsize,CV_8UC3,Scalar(0));
    // Plot mask
    for(unsigned int i = 0; i < signal_count; i++){
        mask.at<Vec3b>(signal_mask[i]/mask.cols,signal_mask[i]%mask.cols)[0] = 0;
        mask.at<Vec3b>(signal_mask[i]/mask.cols,signal_mask[i]%mask.cols)[1] = 0;
        mask.at<Vec3b>(signal_mask[i]/mask.cols,signal_mask[i]%mask.cols)[2] = 255;
    }
    imshow("Mask", mask);
    // Grab 2000 images
    qDebug() << "Start acquiring image to buffer";
    g_surfacebuffercount = 0;
    g_callback_mode = 0;
    g_qtimeObj->start();
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    while(g_surfacebuffercount < g_buffersize-1);
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    int t = g_qtimeObj->elapsed();
    qDebug() << "...Done!";
    qDebug() << "Acquired" << QString::number(g_buffersize) << "images";
    qDebug() << "Time used: " << t/1000.0 << "s";
    // For each frame
    for(int k = 0; k < g_buffersize; k++){
        // Merge two 8-bit char to a 16-bit short
        for(int j = 0;j<OCAM2_PIXELS_RAW_NORMAL;j++){
            g_imageRawNormal[j] = (static_cast<short>((g_surfacebuffer[k][j*2+1])<<8)) | (g_surfacebuffer[k][j*2]);
        }
        ocam2_descramble(g_id, &g_number, g_imageNormal, g_imageRawNormal);
        // Convert to 14 bits data
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            g_imageNormal[i] = g_imageNormal[i] & 0x3fff;
        }
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            img[k][i] = g_imageNormal[i];
        }
        ui->progressBar->setValue(k);
    }

    // Check identical frames
    //    ChangeSection();
    //    int k = 0;
    //    for(int i = 0; i < g_buffersize - 1; i++){
    //        int p = 0;
    //        for(int j = 0; j < OCAM2_PIXELS_IMAGE_NORMAL; j++){
    //            if(img[i][j] != img[i+1][j]){
    //                p = 1;
    //            }
    //        }
    //        if( p == 0){
    //            cout << dec << i << " & " << i+1 << " same" << endl;
    //            k++;
    //        }
    //    }
    //    cout << dec << k << endl;

    // Subtraction of dark image
    for(int j = 0; j < frames; j++){
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            img[j][i] = img[j][i] - g_imageNormalDark[i];
            if(img[j][i] < 0)
                img[j][i] = 0;
        }
    }
    // Copy mask.dat file
    QString outputfilename = "/acqui/data/" + QString::number(frames) + "_mask.dat";
    if(QFile::exists(outputfilename)){
        QFile::remove(outputfilename);
    }
    QFile::copy(maskfilename, outputfilename);
    FILE *pFile;
    string filepath;
    filepath = "/acqui/data/" + to_string(frames) + "_mask.dat";
    char * filename = new char [filepath.length()+1];
    std::strcpy(filename,filepath.c_str());
    pFile = fopen(filename,"wb");
    unsigned int uint[1] = {frames};
    fwrite(filename,4,1,pFile);
    short buffer[signal_count];
    for(unsigned int j = 0; j < frames; j++){
        for(int i = 0; i < signal_count; i++){
            buffer[i] = img[j][signal_mask[i]];
        }
        fwrite(buffer,2,signal_count,pFile);
        ui->progressBar->setValue(j + 1);
    }
    cout << filename << " saved." << endl;
    //    // Save output data
    //    QFile output(outputfilename);
    //    if(output.open(QIODevice::Append)){
    //        QDataStream out(&output);
    //        out << static_cast<quint32>(frames);
    //        for(unsigned int j = 0; j < frames; j++){
    //            for(int i = 0; i < signal_count; i++){
    //                out << static_cast<qint16>(img[j][signal_mask[i]]);
    //            }
    //            ui->progressBar->setValue(j + 1);
    //        }
    //    }
    //    output.close();
    //    qDebug() << output << "saved.";
}

// Load Set Button
void MainWindow::on_LoadSet_PB_clicked()
{
    // Initialization
    unsigned int frames = 0;
    int gain = 0;
    unsigned int signal_count;
    vector<unsigned int> signal_mask;
    short thresh = static_cast<short>(sqrt(g_Sigma2Read) * 5);
    // Loading
    QString fileName = QFileDialog::getOpenFileName(this,"Open file",g_data_path,"Files (*.dat)");
    if(fileName != ""){
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)){
            QMessageBox::warning(this,"Warning","Can not open.",QMessageBox::Yes);
        }else{
            QDataStream in(&file);
            in.setByteOrder(QDataStream::LittleEndian);
            // Read and check the header
            qint16 magic;
            qint16 qgain;
            quint32 data;
            qint16 val;
            in >> magic;
            if(magic != 0xff)
                QMessageBox::warning(this,"Warning","Bad file format", QMessageBox::Ok);
            else{
                in.setVersion(QDataStream::Qt_5_11);
                in >> qgain;
                gain = qgain;
                // Read the mask
                in >> data;
                signal_count = data;
                signal_mask.resize(signal_count);
                for(unsigned int i = 0; i < signal_count; i++){
                    in >> data;
                    signal_mask[i] = data;
                }
                // Read frames
                in >> data;
                frames = data;
                g_MaskSetFrames = frames;
                vector<vector <short>> img(frames, vector<short>(OCAM2_PIXELS_IMAGE_NORMAL,0));
                // Create 16-bits MAT
                Mat png(g_imgsize,g_imgsize,CV_16UC1,Scalar(0));
                // Assign value for signal pixels
                for(unsigned int j = 0; j < frames; j++){
                    for(unsigned int i = 0; i < signal_count; i++){
                        in >> val;
                        //                        cout << double(val)*double(gain)/double(g_gain_value) << "    " << gain + thresh << endl;
                        //                        if(double(val)*double(gain)/double(g_gain_value) < double(gain + thresh)){
                        //                            img[j][signal_mask[i]] = 0;
                        //                        }else if(double(val)*double(gain)/double(g_gain_value) < double(2*gain + thresh)){
                        //                            img[j][signal_mask[i]] = thresh;
                        //                        }else{
                        //                            img[j][signal_mask[i]] = val;
                        //                        }
                        if(val < gain + thresh){
                            img[j][signal_mask[i]] = 0;
                        }else if(val< 2*gain + thresh){
                            img[j][signal_mask[i]] = thresh;
                        }else{
                            img[j][signal_mask[i]] = val;
                        }
                    }
                    for (int m = 0; m < png.rows; m++){
                        for(int n = 0; n < png.cols; n++){
                            png.at<unsigned short>(m,n) = static_cast<unsigned short>(img[j][m*240+n]);
                            //                            // Blue for PC pixels
                            //                            if(img[j][m*240+n] == thresh){
                            //                                png.at<Vec3s>(m,n)[0] = static_cast<short>(thresh);
                            //                                png.at<Vec3s>(m,n)[1] = 0;
                            //                                png.at<Vec3s>(m,n)[2] = 0;
                            //                            }
                            //                            //  Red for IMG pixels
                            //                            else if(img[j][m*240+n] > thresh){
                            //                                png.at<Vec3s>(m,n)[0] = static_cast<short>(img[j][m*240+n]);
                            //                                png.at<Vec3s>(m,n)[1] = static_cast<short>(img[j][m*240+n]);
                            //                                png.at<Vec3s>(m,n)[2] = static_cast<short>(img[j][m*240+n]);
                            //                            }
                        }
                    }
                    string filepath = "/acqui/maskset/" + to_string(j+1) + ".png";
                    imwrite(filepath,png);
                }
            }
        }
        file.close();
    }
    qDebug() << fileName << "loaded.";
}

// PTC Button
void MainWindow::on_PTC_PB_clicked()
{
    // Initialization
    unsigned long frames = 1000;
    // Grab images
    // Start Acquisitions for this channel.
    g_status = McSetParamInt(g_hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
    if(g_status != MC_OK){
        test_error();
    }
    ui->spinBox->setMaximum(g_statistics_try);
    // Allocate memory for images
    // Raw grey value
    short **Xshort = new short*[frames];
    for(int j = 0; j < frames; j++){
        Xshort[j] = new short[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    float **X = new float*[frames];
    for(int j = 0; j < frames; j++){
        X[j] = new float[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    // Grabbed images stored in "images" (240*240)
    for(int j = 0; j < frames; j++){
        g_status = McSetParamInt(g_hChannel, MC_ForceTrig, MC_ForceTrig_TRIG);
        if(g_status != MC_OK){
            test_error();
        }
        ocam2_descramble(g_id, &g_number, Xshort[j], g_imageRawNormal);
        usleep(500);
    }
    // Modifie the 4th and 5th register
    for(int j = 0; j < frames; j++){
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            // 4th register
            if((i/240 < 120) && (i%240 >= 180) && (i%240 < 240)){
                Xshort[j][i] = Xshort[j][i-60];
            }
            if((i/240 > 119) && (i/240 < 240) && (i%240 < 60)){
                Xshort[j][i] = Xshort[j][i+60];
            }
        }
    }
    //    short disp[OCAM2_PIXELS_IMAGE_NORMAL];
    //    pixel_correction(Xshort[50],disp);
    //    display(disp);
    // Assign value for X
    for(int j = 0; j < frames; j++){
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            X[j][i] = Xshort[j][i] & 0x3fff;
            //            cout << "X " <<  j << "    " << i << "    " <<  X[j][i] << endl;
        }
    }
    // Offset
    short OFF[OCAM2_PIXELS_IMAGE_NORMAL];
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        OFF[i] = g_imageNormalDark[i];
        //        cout << OFF[i] << endl;
    }
    // Signal after subtraction of offset
    float **S = new float*[frames];
    for(int j = 0; j < frames; j++){
        S[j] = new float[OCAM2_PIXELS_IMAGE_NORMAL];
    }
    for(int j = 0; j < frames; j++){
        // Assign value for S
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            S[j][i] = X[j][i] - OFF[i];
        }
    }
    // Mean value of S(j)i
    float M[OCAM2_PIXELS_IMAGE_NORMAL] = {0};
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        for(int j = 0; j < frames; j++){
            M[i] += S[j][i];
        }
        M[i] = M[i] / frames;
    }
    // σ²Total
    float sigma2Total = 0;
    for(int j = 0; j < frames; j++){
        for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
            sigma2Total += pow(S[j][i] - M[i], 2);
        }
    }
    sigma2Total = sigma2Total / ((frames -1) * OCAM2_PIXELS_IMAGE_NORMAL);
    // SignalDN
    float signalDN = 0;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        for(int j = 0; j < frames; j++){
            signalDN += S[j][i];
        }
    }
    signalDN = signalDN / (frames * OCAM2_PIXELS_IMAGE_NORMAL);
    cout << "Signal DN = " << signalDN << endl;
    // σ²Read
    float sigma2Read = 0;
    for(int i = 0; i < OCAM2_PIXELS_IMAGE_NORMAL; i++){
        for(int j = 0; j < frames - 1; j++){
            sigma2Read += pow(S[j][i]-S[j+1][i],2);
        }
    }
    sigma2Read = sigma2Read / (2 * (frames - 1) * OCAM2_PIXELS_IMAGE_NORMAL);
    // Kadc
    float Kadc = 0;
    Kadc = signalDN / (sigma2Total - sigma2Read);
    cout << "σ²Read DN = " << sigma2Read << endl;
    cout << "σ²Shot DN = " << (sigma2Total - sigma2Read) << endl;
    cout << "Kadc = Signal DN / (σ²Shot) = " << Kadc << endl;
    g_PTC_Signal[g_PTC_num] = signalDN;
    g_PTC_Sigma2Read[g_PTC_num] = sigma2Read;
    g_PTC_Sigma2Shot[g_PTC_num] = sigma2Total - sigma2Read;
    g_PTC_num++;
    cout << dec << g_PTC_num << " / " << g_statistics_try << " done." << endl;
    if(g_PTC_num == g_statistics_try)
        g_PTC_num = 0;
}


/*================= QT GUI BUTTONS END ======================================*/

/*================= QT GUI FUNCTIONS START ======================================*/

void MainWindow::on_BP_Slider_valueChanged(int value)
{
    g_BP = value;
    if(g_BP >= g_WP)
        ui->BP_Slider->setValue(g_WP-1);
    ui->BPvalue_Label->setText(QString::number(g_BP));
    short image[OCAM2_PIXELS_IMAGE_NORMAL];
    pixel_correction(g_imageNormal,image);
    display(image);
}

void MainWindow::on_WP_Slider_valueChanged(int value)
{
    g_WP = value;
    if(g_WP <= g_BP)
        ui->WP_Slider->setValue(g_BP+1);
    ui->WPvalue_Label->setText(QString::number(g_WP));
    short image[OCAM2_PIXELS_IMAGE_NORMAL];
    pixel_correction(g_imageNormal,image);
    display(image);
}

void MainWindow::on_Max_checkBox_stateChanged(int arg1)
{
    if(arg1 == 2){
        ui->Sequence_spinBox->setValue(ui->Sequence_spinBox->maximum());
    }
    if(arg1 == 0){
        ui->Sequence_spinBox->setValue(0);
    }
}

void MainWindow::on_Median_Checkbox_stateChanged(int arg1)
{
    if(arg1 == 2){
        g_median_state = true;
        cout << "median activated." << endl;
        cout << g_median_state << endl;
    }
    if(arg1 == 0){
        g_median_state = false;
        cout << "median desactivated" << endl;
        cout << g_median_state << endl;
    }
}

void MainWindow::on_ReadNoise_Checkbox_stateChanged(int arg1)
{

    if(arg1 == 2){
        g_ReadNoise_state = true;
        cout << "Read Noise suppression activated." << endl;
        cout << g_ReadNoise_state << endl;
    }
    if(arg1 == 0){
        g_ReadNoise_state = false;
        cout << "Read Noise suppression desactivated" << endl;
        cout << g_ReadNoise_state << endl;
    }
}

// Toggle Dark Function
void MainWindow::on_Dark_Checkbox_stateChanged(int arg1)
{
    if(arg1 == 2){
        g_dark_state = true;
        cout << "dark activated." << endl;
    }
    if(arg1 == 0){
        g_dark_state = false;
        cout << "dark desactivated" << endl;
    }
}

// Toggle Flat Function
void MainWindow::on_Flat_Checkbox_stateChanged(int arg1)
{
    if(arg1 == 2){
        g_flat_state = true;
        cout << "flat activated." << endl;
        ui->BP_Slider->setValue(0);
        ui->WP_Slider->setValue(16383);

    }
    if(arg1 == 0){
        g_flat_state = false;
        cout << "flat desactivated" << endl;
    }
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    g_masknum = arg1;
    g_PTC_num = arg1 - 1;
}

void MainWindow::on_Threshold_Slider_valueChanged(int value)
{
    g_threshvalue = value;
    ui->Thresh_Label->setText(QString::number(g_threshvalue) + " % after thresh = " + QString::number(g_afterthresh));
}

void MainWindow::on_Frequency_Slider_valueChanged(int value)
{
    g_display_frequency = value;
    ui->Displayfreq_Label->setText("Display frequency = " + QString::number(g_display_frequency) + " ms");
}

void MainWindow::on_Console_Input_LineEdit_returnPressed()
{
    QString str = ui->Console_Input_LineEdit->text();
    // Avoid accidentally reset gain table.
    if(str == "gain reset" || str == "gain load"){
        str = "gain";
    }
    SerialCommand(str,0);
    ui->Console_Input_LineEdit->clear();
}

void MainWindow::on_GaussianNoise_CheckBox_stateChanged(int arg1)
{
    if(arg1 == 2){
        g_addGaussianNoise_state = true;
        cout << "Gaussian Noise added." << endl;
    }
    if(arg1 == 0){
        g_addGaussianNoise_state = false;
        cout << "Gaussian Noise cancelled." << endl;
    }
}

void MainWindow::on_Test4K_RB_clicked(bool checked)
{
    if(checked){
        g_test4K = true;
        g_status = McSetParamStr(g_hChannel,MC_CamFile,"/home/gcai/LAM/Qt/Ocam/MCL4K");
        // Retrieve channel size information.
        g_status= McGetParamInt(g_hChannel, MC_ImageSizeX, &g_sizeX);
        g_status= McGetParamInt(g_hChannel, MC_ImageSizeY, &g_sizeY);
        g_status= McGetParamInt(g_hChannel, MC_BufferPitch, &g_pitch);
        cout << g_sizeX << " " << g_sizeY << endl;
        cout << "4K" << endl;
    }else{
        g_test4K = false;
        g_status = McSetParamStr(g_hChannel,MC_CamFile,"/home/gcai/LAM/Qt/Ocam/MCL");
        // Retrieve channel size information.
        g_status= McGetParamInt(g_hChannel, MC_ImageSizeX, &g_sizeX);
        g_status= McGetParamInt(g_hChannel, MC_ImageSizeY, &g_sizeY);
        g_status= McGetParamInt(g_hChannel, MC_BufferPitch, &g_pitch);
        cout << g_sizeX << " " << g_sizeY << endl;
        cout << "4K end" << endl;
    }
}

void MainWindow::on_Sampling_Button_toggled(bool checked)
{
    if(checked){
        g_sampling4k = true;
        g_megapixel4k = false;
        ui->MegaPixel_Button->setChecked(false);
    }
}

void MainWindow::on_MegaPixel_Button_toggled(bool checked)
{
    if(checked){
        g_megapixel4k = true;
        g_sampling4k = false;
        ui->Sampling_Button->setChecked(false);
    }
}

/*================= QT GUI FUNCTIONS END ======================================*/

/*================= OTHER FUNCTION START ======================================*/

// Change Section Function
void MainWindow::ChangeSection(){
    cout << "===========================================" << endl;
}

// Convert short array to float vector
vector<float> MainWindow::short2float(short in[], const int length){
    vector<float> out(length);
    for(int i = 0; i < length; i++){
        out[i] = static_cast<float>(in[i]);
    }
    return out;
}


// Test for Voronoi Diagram
typedef std::pair<short,int> mypair;

bool comparator ( const mypair& l, const mypair& r){
    return l.first > r.first;
}

int get_x(int in){
    int x = in % g_imgsize;
    return x;
}

int get_y(int in){
    int y = in / g_imgsize;
    return y;
}

/*================= OTHER FUNCTION END ======================================*/
