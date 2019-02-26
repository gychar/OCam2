#include "qgraphicsviewcustom.h"
#include "g_param.h"

QGraphicsItemCustom* QGraphicsViewCustom::item =  new QGraphicsItemCustom(0,0,0,0);

QGraphicsViewCustom::QGraphicsViewCustom(QWidget *parent) : QGraphicsView (parent){
    setMouseTracking(true);
}

double QGraphicsViewCustom::GetMean(vector<short> img, unsigned int a, unsigned int b, unsigned int c, unsigned d)
{
    unsigned int xx1 = (a < c) ? a : c;
    unsigned int yy1 = (b < d) ? b : d;
    unsigned int xx2 = (xx1 == a) ? c : a;
    unsigned int yy2 = (yy1 == b) ? d : b;
    unsigned int sum = 0;
    unsigned int size = (xx2-xx1)*(yy2-yy1);
    cout << dec << size << " pixels ";
    int length = 0;
    for(unsigned int j = 0; j < yy2-yy1; j++){
        for(unsigned int i = 0; i < xx2-xx1; i++){
            length = g_zoom_full ? g_imgsize : g_mask_x;
            sum += img[xx1+i+(yy1+j)*length];
        }
    }
    double ret = double(sum)/double(size);
    g_zoom_mean = ret;
    g_zoom_mean_done = true;
    cout << "mean = " << ret << endl;
    return ret;
}

double QGraphicsViewCustom::GetMedian(vector<short> img, unsigned int a, unsigned int b, unsigned int c, unsigned d)
{
    unsigned int xx1 = (a < c) ? a : c;
    unsigned int yy1 = (b < d) ? b : d;
    unsigned int xx2 = (xx1 == a) ? c : a;
    unsigned int yy2 = (yy1 == b) ? d : b;
    unsigned int size = (xx2-xx1)*(yy2-yy1);
    cout << dec << size << " pixels ";
    int length = 0;
    vector<double> rect;
    for(unsigned int j = 0; j < yy2-yy1; j++){
        for(unsigned int i = 0; i < xx2-xx1; i++){
            length = g_zoom_full ? g_imgsize : g_mask_x;
            rect.push_back(img[xx1+i+(yy1+j)*length]);
        }
    }
    double ret = g_maths.Median(rect);
    g_zoom_median = ret;
    g_zoom_median_done = true;
    cout << "median = " << ret << endl;
    return ret;
}

double QGraphicsViewCustom::GetSD(vector<short> img, unsigned int a, unsigned int b, unsigned int c, unsigned d)
{
    unsigned int xx1 = (a < c) ? a : c;
    unsigned int yy1 = (b < d) ? b : d;
    unsigned int xx2 = (xx1 == a) ? c : a;
    unsigned int yy2 = (yy1 == b) ? d : b;
    unsigned int size = (xx2-xx1)*(yy2-yy1);
    cout << dec << size << " pixels ";
    int length = 0;
    vector<double> rect;
    for(unsigned int j = 0; j < yy2-yy1; j++){
        for(unsigned int i = 0; i < xx2-xx1; i++){
            length = g_zoom_full ? g_imgsize : g_mask_x;
            rect.push_back(img[xx1+i+(yy1+j)*length]);
        }
    }
    double ret = g_maths.StandardDeviation(rect);
    g_zoom_sd = ret;
    g_zoom_sd_done = true;
    cout << "sd = " << ret << endl;
    return ret;
}

void QGraphicsViewCustom::mouseMoveEvent(QMouseEvent *event){
    this->setCursor(Qt::CrossCursor);
    if(!g_test4K){
        x = unsigned(event->x()/4);
        y = unsigned(event->y()/4);
    }else{
        x = unsigned(mapToScene(event->pos()).x());
        y = unsigned(mapToScene(event->pos()).y());
    }
    g_zoom_x = x;
    g_zoom_y = y;
    QGraphicsView::mouseMoveEvent(event);
    //    cout << dec << x << " " << y << endl;
}

void QGraphicsViewCustom::mousePressEvent(QMouseEvent *event)
{
    this->scene()->update();
    if(!g_test4K){
        x1 = unsigned(event->x());
        y1 = unsigned(event->y());
        cout << dec << "press : x = " << x1/4 << " y = " << y1/4 << endl;
    }else{
        x1 = unsigned(mapToScene(event->pos()).x());
        y1 = unsigned(mapToScene(event->pos()).y());
        int index = x1 + y1 * 8448;
        QPoint pt(x1,y1);
        g_image4K->setPixel(pt,qRgb(255,0,255));
        g_scene_4k->clear();
        g_scene_4k->addPixmap(QPixmap::fromImage(*g_image4K));
        cout << dec << "pixel : x = " << x1 << " y = " << y1 << " value = " << hex << g_img4K_pixel_val[index] << endl;
    }
    g_zoom_mean_done = false;
}

void QGraphicsViewCustom::mouseReleaseEvent(QMouseEvent *event)
{
    if(!g_test4K){
        x2 = unsigned(event->x());
        y2 = unsigned(event->y());
        item = new QGraphicsItemCustom(x1,y1,x2,y2);
        this->scene()->addItem(item);
        GetMean(g_zoom_full? g_imgNormal_vector : g_register_zoom_vector, x1/4,y1/4,x2/4,y2/4);
        GetMedian(g_zoom_full? g_imgNormal_vector : g_register_zoom_vector, x1/4,y1/4,x2/4,y2/4);
        GetSD(g_zoom_full? g_imgNormal_vector : g_register_zoom_vector, x1/4,y1/4,x2/4,y2/4);
        cout << dec << "release : x = " << x2/4 << " y = " << y2/4 << endl;
    }else{

    }
}
