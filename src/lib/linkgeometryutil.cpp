//
// Copyright 2014 wkhtmltopdf authors
//
// This file is part of wkhtmltopdf.
//
// wkhtmltopdf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// wkhtmltopdf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with wkhtmltopdf.  If not, see <http://www.gnu.org/licenses/>.

#include "linkgeometryutil.h"
#include <QTextStream>
#include <QWebElement>
#include <QWebFrame>

// the JSON string clean up function borrowed from QJson project
static QString sanitizeString( QString str )
{
    str.replace( QLatin1String( "\\" ), QLatin1String( "\\\\" ) );

    // escape unicode chars
    QString result;
    const ushort* unicode = str.utf16();
    unsigned int i = 0;

    while ( unicode[ i ] ) {
        if ( unicode[ i ] < 128 ) {
            result.append( QChar( unicode[ i ] ) );
        }
        else {
            QString hexCode = QString::number( unicode[ i ], 16 ).rightJustified( 4,
                                                                                  QLatin1Char('0') );

            result.append( QLatin1String ("\\u") ).append( hexCode );
        }
        ++i;
    }
    str = result;

    str.replace( QLatin1String( "\"" ), QLatin1String( "\\\"" ) );
    str.replace( QLatin1String( "\b" ), QLatin1String( "\\b" ) );
    str.replace( QLatin1String( "\f" ), QLatin1String( "\\f" ) );
    str.replace( QLatin1String( "\n" ), QLatin1String( "\\n" ) );
    str.replace( QLatin1String( "\r" ), QLatin1String( "\\r" ) );
    str.replace( QLatin1String( "\t" ), QLatin1String( "\\t" ) );

    return str;
}

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

    QTextStream(stdout) << "\"LinkText\":\"" << sanitizeString(plainText) << "\",";

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
    int maxX = 0, maxY = 0;
    QStringList list = coords.split(",");
    int minX = list.at(0).toInt();
    int minY = list.at(1).toInt();
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
    QString normalizeShape = element.attribute("shape");
    QString coords = element.attribute("coords");

    if (normalizeShape.isNull() || normalizeShape.isEmpty()) {
        // treat missing or empty shape attribute as poly
        elementRect = processPloyShapeCoords(coords, rect);
    } else {
        normalizeShape = normalizeShape.toLower();
        if (normalizeShape == "poly") {
            elementRect = processPloyShapeCoords(coords, rect);
        } else if (normalizeShape == "rect") {
            elementRect = processRectShapeCoords(coords, rect);
        } else if (normalizeShape == "circle") {
            elementRect = processCircleShapeCoords(coords, rect);
        }
    }

    QString href = element.attribute("href");
    QString altText = element.attribute("alt");
    outputElementGeometry(href, altText, elementRect);
}

void LinkGeometryUtil::outputGeometry(const QWebFrame * frame) {
    QWebElementCollection collection = frame->findAllElements("a, img[usemap]");
    QTextStream(stdout) << "{\"Geometries\":[";
    int elementCount = 0;
    foreach(QWebElement e, collection) {
        if (e.tagName().toLower() == "a") {
            if (elementCount > 0) {
                QTextStream(stdout) << "," << endl;
            }
            QString href = e.attribute("href");
            QString text = e.toPlainText();
            QRect rect = e.geometry();
            if (rect.height() < 1) {
                QWebElement c = e.firstChild();
                if (!c.isNull()) {
                    rect.setHeight(c.geometry().height());
                    text = c.attribute("alt");
                }
            }

            outputElementGeometry(href, text, rect);
            elementCount++;
        } else if (e.tagName().toLower() == "img") {
            QString mapName = e.attribute("usemap");
            if (mapName.startsWith("#"))
                mapName = mapName.right(mapName.size() - 1);
            QWebElement map = frame->findFirstElement("map[name=" + mapName + "]");
            if (map.isNull()) {
                continue;
            }
            QWebElement child = map.firstChild();
            if (!child.isNull()) {
                if (elementCount > 0) {
                    QTextStream(stdout) << "," << endl;
                }
                processMapShape(child, e.geometry());

                QWebElement current = child;
                QWebElement next;
                while (!(next = current.nextSibling()).isNull()) {
                    QTextStream(stdout) << "," << endl;
                    processMapShape(next, e.geometry());
                    current = next;
                }
                elementCount++;
            }
        }

    }
    QTextStream(stdout) << "], \"StatusID\":0, \"StatusDesc\":\"Success\"}" << endl;
}
