#include "flprogTca8418.h"

FLProgTca8418::FLProgTca8418(uint8_t address, uint8_t bus, uint8_t rows, uint8_t columns)
{
  RT_HW_Base.i2cSetParam(_device, address, bus, RT_HW_I2C_SPEED);
  privateCreate(rows, columns);
}

FLProgTca8418::FLProgTca8418(uint8_t address, uint8_t bus, uint8_t rows, uint8_t columns, uint8_t expander, uint8_t channel)
{
  RT_HW_Base.i2cSetParam(_device, address, bus, RT_HW_I2C_SPEED, expander, channel);
  privateCreate(rows, columns);
}

void FLProgTca8418::setReqestPerion(uint32_t period)
{
  if (_reqestPeriod == period)
  {
    return;
  }
  _lastRequestTime = millis();
  _reqestPeriod = period;
}

void FLProgTca8418::privateCreate(uint8_t rows, uint8_t columns)
{
  if (rows > 8)
  {
    _rows = 8;
  }
  else
  {
    _rows = rows;
  }
  if (columns > 10)
  {
    _columns = 10;
  }
  else
  {
    _columns = columns;
  }
  _status = FLPROG_NOT_REDY_STATUS;
}

uint8_t FLProgTca8418::readRegister(uint8_t reg)
{
  RT_HW_Base.i2cWrite(_device, reg);
  RT_HW_Base.i2cRead(_device);
  return _device.bf8;
}

void FLProgTca8418::writeRegister(uint8_t reg, uint8_t value)
{
  uint8_t data[2];
  data[0] = reg;
  data[1] = value;
  RT_HW_Base.i2cWriteArr(_device, data, 2);
}

void FLProgTca8418::pool()
{
  if (_status == FLPROG_NOT_REDY_STATUS)
  {
    RT_HW_Base.i2cInitDevice(_device);
    if (!_device.link)
    {
      return;
    }
    if (_device.status == 1)
    {
      _status = FLPROG_WAIT_I2C_FIND_ADDRESS;
      return;
    }
    else
    {
      return;
    }
  }
  if (_status == FLPROG_WAIT_I2C_FIND_ADDRESS)
  {
    RT_HW_Base.i2cFindAdr(_device);
    if (!_device.link)
    {
      return;
    }
    if (_device.codeErr)
    {
      _status = FLPROG_WAIT_I2C_REFIND_ADDRES_PAUSE;
      _pauseStartTime = millis();
      return;
    }
    else
    {
      _status = FLPROG_WAIT_I2C_DEVICE_INIT;
      return;
    }
  }

  if (_status == FLPROG_WAIT_I2C_REFIND_ADDRES_PAUSE)
  {
    if (flprog::isTimer(_pauseStartTime, 1000))
    {
      _status = FLPROG_WAIT_I2C_FIND_ADDRESS;
    }
    else
    {
      return;
    }
  }
  if (_status == FLPROG_WAIT_I2C_DEVICE_INIT)
  {
    init();
    return;
  }
  readData();
  if (_device.codeErr)
  {
    _status = FLPROG_NOT_REDY_STATUS;
    return;
  }
}

uint8_t FLProgTca8418::getEvent()
{
  uint8_t event = readRegister(FLPROG_TCA8418_REG_KEY_EVENT_A);
  return event;
}

uint8_t FLProgTca8418::flush()
{
  uint8_t count = 0;
  while (getEvent() != 0)
  {
    count++;
  }
  readRegister(FLPROG_TCA8418_REG_GPIO_INT_STAT_1);
  readRegister(FLPROG_TCA8418_REG_GPIO_INT_STAT_2);
  readRegister(FLPROG_TCA8418_REG_GPIO_INT_STAT_3);
  //  clear INT_STAT register
  writeRegister(FLPROG_TCA8418_REG_INT_STAT, 3);
  return count;
}

void FLProgTca8418::init()
{

  //  GPIO
  //  set default all GIO pins to INPUT
  writeRegister(FLPROG_TCA8418_REG_GPIO_DIR_1, 0x00);
  writeRegister(FLPROG_TCA8418_REG_GPIO_DIR_2, 0x00);
  writeRegister(FLPROG_TCA8418_REG_GPIO_DIR_3, 0x00);

  //  add all pins to key events
  writeRegister(FLPROG_TCA8418_REG_GPI_EM_1, 0xFF);
  writeRegister(FLPROG_TCA8418_REG_GPI_EM_2, 0xFF);
  writeRegister(FLPROG_TCA8418_REG_GPI_EM_3, 0xFF);

  //  set all pins to FALLING interrupts
  writeRegister(FLPROG_TCA8418_REG_GPIO_INT_LVL_1, 0x00);
  writeRegister(FLPROG_TCA8418_REG_GPIO_INT_LVL_2, 0x00);
  writeRegister(FLPROG_TCA8418_REG_GPIO_INT_LVL_3, 0x00);

  //  add all pins to interrupts
  writeRegister(FLPROG_TCA8418_REG_GPIO_INT_EN_1, 0xFF);
  writeRegister(FLPROG_TCA8418_REG_GPIO_INT_EN_2, 0xFF);
  writeRegister(FLPROG_TCA8418_REG_GPIO_INT_EN_3, 0xFF);

  //  MATRIX
  //  skip zero size matrix
  if ((_rows != 0) && (_columns != 0))
  {
    uint8_t mask = 0x00;
    for (int r = 0; r < _rows; r++)
    {
      mask <<= 1;
      mask |= 1;
    }
    writeRegister(FLPROG_TCA8418_REG_KP_GPIO_1, mask);

    mask = 0x00;
    for (int c = 0; c < _columns && c < 8; c++)
    {
      mask <<= 1;
      mask |= 1;
    }
    writeRegister(FLPROG_TCA8418_REG_KP_GPIO_2, mask);

    if (_columns > 8)
    {
      if (_columns == 9)
        mask = 0x01;
      else
        mask = 0x03;
      writeRegister(FLPROG_TCA8418_REG_KP_GPIO_3, mask);
    }
  }
  flush();
  _status = FLPROG_READY_STATUS;
}

uint8_t FLProgTca8418::available()
{
  uint8_t eventCount = readRegister(FLPROG_TCA8418_REG_KEY_LCK_EC);
  eventCount &= 0x0F; //  lower 4 bits only
  return eventCount;
}

bool FLProgTca8418::canReqest()
{
  if (_reqestPeriod == 0)
  {
    return true;
  }
  if (flprog::isTimer(_lastRequestTime, _reqestPeriod))
  {
    _lastRequestTime = millis();
    return true;
  }
  return false;
}

void FLProgTca8418::readData()
{
  if (!canReqest())
  {
    return;
  }
  if (available() == 0)
  {
    return;
  }
  uint16_t k = getEvent();
  bool position = false;
  if (k & 0x80)
  {
    position = true;
  }
  k &= 0x7F;
  k--;
  uint8_t row = k / 10;
  uint8_t col = k % 10;
  if (row > 7)
  {
    return;
  }
  if (col > 9)
  {
    return;
  }
  _buttons[row][col] = position;
}

bool FLProgTca8418::buttonState(uint8_t row, uint8_t col)
{
  if (row > 7)
  {
    return false;
  }
  if (col > 9)
  {
    return false;
  }
  return _buttons[row][col];
}