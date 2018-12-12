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

void QGraphicsViewCustom::mouseMoveEvent(QMouseEvent *event){
    this->setCursor(Qt::CrossCursor);
    x = unsigned(event->x()/4);
    y = unsigned(event->y()/4);
    g_zoom_x = x;
    g_zoom_y = y;
    QGraphicsView::mouseMoveEvent(event);
}

void QGraphicsViewCustom::mousePressEvent(QMouseEvent *event)
{
    this->scene()->update();
    x1 = unsigned(event->x());
    y1 = unsigned(event->y());
    g_zoom_mean_done = false;
    cout << dec << "press : x = " << x1/4 << " y = " << y1/4 << endl;
}

void QGraphicsViewCustom::mouseReleaseEvent(QMouseEvent *event)
{
    x2 = unsigned(event->x());
    y2 = unsigned(event->y());
    item = new QGraphicsItemCustom(x1,y1,x2,y2);
    this->scene()->addItem(item);
    GetMean(g_zoom_full? g_imgNormal_vector : g_register_zoom_vector, x1/4,y1/4,x2/4,y2/4);
    cout << dec << "release : x = " << x2/4 << " y = " << y2/4 << endl;
}
