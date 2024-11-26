#include <QApplication>
#include <iostream>
#include "Widgets/TriangleWidget.h"
#include "Widgets/MixWidget.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    
    if(argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <m> or <t>" << std::endl;
        return 1;
    }

    if(argv[1][0] == 't') // Triangle without color interpolation
    {
        TriangleWidget triangle;
        triangle.resize(640, 480);
        triangle.show();
        return app.exec();
    }
    else if(argv[1][0] == 'm') // GLTF model with depth peeling
    {
        MixWidget mix;
        mix.resize(640,480);
        mix.show();
        return app.exec();
    }
    else
    {
        std::cerr << "Usage: " << argv[0] << " <m> or <t>" << std::endl;
        return 1;
    }

}
