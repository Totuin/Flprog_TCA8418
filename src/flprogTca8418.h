#pragma once
#include "flprogUtilites.h"
#include "flprogTca8418Registers.h"

/** Pin IDs for matrix rows/columns */
enum
{
  FLPROG_TCA8418_ROW0, // Pin ID for row 0
  FLPROG_TCA8418_ROW1, // Pin ID for row 1
  FLPROG_TCA8418_ROW2, // Pin ID for row 2
  FLPROG_TCA8418_ROW3, // Pin ID for row 3
  FLPROG_TCA8418_ROW4, // Pin ID for row 4
  FLPROG_TCA8418_ROW5, // Pin ID for row 5
  FLPROG_TCA8418_ROW6, // Pin ID for row 6
  FLPROG_TCA8418_ROW7, // Pin ID for row 7
  FLPROG_TCA8418_COL0, // Pin ID for column 0
  FLPROG_TCA8418_COL1, // Pin ID for column 1
  FLPROG_TCA8418_COL2, // Pin ID for column 2
  FLPROG_TCA8418_COL3, // Pin ID for column 3
  FLPROG_TCA8418_COL4, // Pin ID for column 4
  FLPROG_TCA8418_COL5, // Pin ID for column 5
  FLPROG_TCA8418_COL6, // Pin ID for column 6
  FLPROG_TCA8418_COL7, // Pin ID for column 7
  FLPROG_TCA8418_COL8, // Pin ID for column 8
  FLPROG_TCA8418_COL9  // Pin ID for column 9
};

class FLProgTca8418 : public AbstractFLProgClass
{
public:
  FLProgTca8418(uint8_t address, uint8_t bus, uint8_t rows, uint8_t columns);
  void pool();
  uint8_t flush();
  uint8_t getEvent();
  uint8_t available();
  bool buttonState(uint8_t row, uint8_t column);

protected:
  void init();
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);
  void readData();
  RT_HW_STRUCT_I2C_DEV _device;
  uint8_t _rows;
  uint8_t _columns;
  uint8_t _address;
  uint8_t _bus;
  bool _buttons[8][10] = {false}; // Assuming maximum of 8 rows and 10 columns
};