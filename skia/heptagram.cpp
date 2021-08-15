#include "include/core/SkSurface.h"
#include "include/core/SkPath.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"

void draw(SkCanvas *canvas)
{
  const SkScalar scale = 256.0f;
  const SkScalar R = 0.45f * scale;
  const SkScalar TAU = 6.2831853f;
  SkPath path;
  path.moveTo(R, 0.0f);
  for (int i = 1; i < 7; ++i)
  {
    SkScalar theta = 3 * i * TAU / 7;
    path.lineTo(R * cos(theta), R * sin(theta));
  }
  path.close();
  SkPaint p;
  p.setAntiAlias(true);
  canvas->clear(SK_ColorWHITE);
  canvas->translate(0.5f * scale, 0.5f * scale);
  canvas->drawPath(path, p);
}

int main(int argc, char *const argv[])
{
  // hard coded example program parameters
  const char filePath[] = "./outputs/heptagram.png";
  int width = 256;
  int height = 256;

  // create canvas to draw on
  sk_sp<SkSurface> rasterSurface = SkSurface::MakeRasterN32Premul(width, height);
  SkCanvas *canvas = rasterSurface->getCanvas();

  draw(canvas);

  // make a PNG encoded image using the canvas
  sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
  if (!img)
  {
    return 1;
  }
  sk_sp<SkData> png(img->encodeToData());
  if (!png)
  {
    return 1;
  }

  // write the data to the file specified by filePath
  SkFILEWStream out(filePath);
  (void)out.write(png->data(), png->size());
  return 0;
}
