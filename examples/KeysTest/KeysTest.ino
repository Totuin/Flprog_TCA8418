#include "flprogTca8418.h"

FLProgTca8418 keyboad(0x30, 0, 8, 10);

void setup()
{
}

void loop()
{
  keyboad.pool();
}