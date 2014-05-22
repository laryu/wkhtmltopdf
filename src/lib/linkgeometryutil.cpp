#include "linkgeometryutil.h"
//#include <cstdio>
#include <QTextStream>
#include <QWebElement>
#include <QWebFrame>

LinkGeometryUtil::LinkGeometryUtil() {
}

void LinkGeometryUtil::outputErrorMessage(int statusCode, const char * errorMsg) {
    QTextStream(stdout) << "{\"Geometries\":[], \"StatusID\":"
                        << statusCode
                        <<", \"StatusDesc\":\""
                        << errorMsg << "\"}" << endl;
}

static void outputElementGeometry(QString& href, QString& plainText, QRect rect) {
    QTextStream(stdout) << "{\"LinkURL\":\"" << href << "\",";

    // replace \n char to space in text
    if (!plainText.isNull())
        plainText = plainText.replace("\n", " ");
    QTextStream(stdout) << "\"LinkText\":\"" << plainText << "\",";

    QTextStream(stdout) << "\"LinkGeo\":\"" << rect.y() << "," << rect.x()
                        << "," << rect.y() << "," << rect.x() + rect.width()
                        << "," << rect.y() + rect.height()  << "," << rect.x() + rect.width()
                        << "," << rect.y() + rect.height()  << "," << rect.x()
                        << "," << rect.y() << "," << rect.x() << "\"}";
}

/*
x1,y1,x2,y2
Specifies the coordinates of the left, top, right, bottom corner of the rectangle (for shape="rect")

x,y,radius
Specifies the coordinates of the circle center and the radius (for shape="circle")

x1,y1,x2,y2,..,xn,yn
Specifies the coordinates of the edges of the polygon. If the first and last coordinate pairs are not the same,
the browser will add the last coordinate pair to close the polygon (for shape="poly")
*/
static QRect processPloyShapeCoords(QString& coords, QRect& rect) {
    int minX = 0, minY = 0,  maxX = 0, maxY = 0;
    QStringList list = coords.split(",");
    for (int i = 0; i < list.size(); i=i+2) {
        int x = list.at(i).toInt();
        int y = list.at(i+1).toInt();
        minX = minX <= x ? minX : x;
        maxX = maxX < x ? x : maxX;
        minY = minY <= y ? minY : y;
        maxY = maxY < y ? y : maxY;
    }

    QRect areaRect(rect.x() + minX, rect.y() + minY, maxX - minX, maxY - minY);
    return areaRect;
}

static QRect processRectShapeCoords(QString& coords, QRect& rect) {
    int minX = 0, minY = 0,  maxX = 0, maxY = 0;
    QStringList list = coords.split(",");
    minX = list.at(0).toInt();
    minY = list.at(1).toInt();
    maxX = list.at(2).toInt();
    maxY = list.at(3).toInt();

    QRect areaRect(rect.x() + minX, rect.y() + minY, maxX - minX, maxY - minY);
    return areaRect;
}

static QRect processCircleShapeCoords(QString& coords, QRect& rect) {
    int x = 0, y = 0, r = 0;
    QStringList list = coords.split(",");
    x = list.at(0).toInt();
    y = list.at(1).toInt();
    r = list.at(2).toInt();

    QRect areaRect(rect.x() + x - r, rect.y() + y - r, 2*r, 2*r);
    return areaRect;
}

static void processMapShape(QWebElement& element, QRect rect) {
    QRect elementRect;
    QString normalizeShape = element.attribute("shape").toLower();
    QString coords = element.attribute("coords");
    if (normalizeShape == "poly") {
        elementRect = processPloyShapeCoords(coords, rect);
    } else if (normalizeShape == "rect") {
        elementRect = processRectShapeCoords(coords, rect);
    } else if (normalizeShape == "circle") {
        elementRect = processRectShapeCoords(coords, rect);
    }

    QString href = element.attribute("href");
    QString altText = element.attribute("alt");
    outputElementGeometry(href, altText, elementRect);
}

void LinkGeometryUtil::outputGeometry(const QWebFrame * frame) {
    QWebElementCollection collection = frame->findAllElements("a, img[usemap]");
    QTextStream(stdout) << "Found total links: " << collection.count() << endl;
    QTextStream(stdout) << "{\"Geometries\":[";
    int elementCount = 0;
    foreach(QWebElement e, collection) {
        if (elementCount > 0) {
            QTextStream(stdout) << "," << endl;
        }

        if (e.tagName().toLower() == "a") {
            QString href = e.attribute("href");
            QString text = e.toPlainText();
            outputElementGeometry(href, text, e.geometry());
        } else if (e.tagName().toLower() == "img") {
            QString mapName = e.attribute("usemap");
            if (mapName.startsWith("#"))
                mapName = mapName.right(mapName.size() - 1);
            QWebElement map = frame->findFirstElement("map[name=" + mapName + "]");
            QWebElement child = map.firstChild();
            if (!child.isNull())
                processMapShape(child, e.geometry());
            QWebElement current = child;
            QWebElement next;
            while (!(next = current.nextSibling()).isNull()) {
                QTextStream(stdout) << "," << endl;
                processMapShape(next, e.geometry());
                current = next;
            }

        }
        elementCount++;
    }
    QTextStream(stdout) << "], \"StatusID\":0, \"StatusDesc\":\"Success\"}" << endl;
}
