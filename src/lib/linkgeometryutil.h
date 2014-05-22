#ifndef LINKGEOMETRYUTIL_H
#define LINKGEOMETRYUTIL_H

class QWebFrame;
class LinkGeometryUtil
{
public:
    LinkGeometryUtil();

    static void outputErrorMessage(int statusCode, const char * errorMsg);
    static void outputGeometry(const QWebFrame * frame);
};

#endif // LINKGEOMETRYUTIL_H
