#pragma once
#include <cstdint>

namespace Core {

//regions
static constexpr uint16_t FT = 1000;
static constexpr uint16_t FD = 2000;
static constexpr uint16_t CD = 3000;
static constexpr uint16_t BD = 4000;

//detectors
static constexpr uint8_t FTOF = 12;
static constexpr uint8_t CTOF = 4;
static constexpr uint8_t CND = 3;
static constexpr uint8_t CVT = 5;
static constexpr uint8_t DC = 6;
static constexpr uint8_t ECAL = 7;
static constexpr uint8_t FTCAL = 10;
static constexpr uint8_t FTTRK = 13;
static constexpr uint8_t FTHODO = 11;
static constexpr uint8_t HTCC = 15;
static constexpr uint8_t LTCC = 16;
static constexpr uint8_t BMT = 1;
static constexpr uint8_t FMT = 8;
static constexpr uint8_t RF = 17;
static constexpr uint8_t RICH = 18;
static constexpr uint8_t RTPC = 19;
static constexpr uint8_t HEL = 20;
static constexpr uint8_t BAND = 21;

//layers
static constexpr uint8_t FTOF1A = 1;
static constexpr uint8_t FTOF1B = 2;
static constexpr uint8_t FTOF2 = 3;

static constexpr uint8_t CNDOFF = 150;
static constexpr uint8_t CND1 = 151;
static constexpr uint8_t CND2 = 152;
static constexpr uint8_t CND3 = 153;

static constexpr uint8_t PCAL = 1;
static constexpr uint8_t ECIN = 4;
static constexpr uint8_t ECOUT = 7;

static constexpr uint8_t DC1 = 6;
static constexpr uint8_t DC2 = 12;
static constexpr uint8_t DC3 = 18;
static constexpr uint8_t DC4 = 24;
static constexpr uint8_t DC5 = 30;
static constexpr uint8_t DC6 = 36;

static constexpr uint8_t CVT1 = 1;
static constexpr uint8_t CVT2 = 2;
static constexpr uint8_t CVT3 = 3;
static constexpr uint8_t CVT4 = 4;
static constexpr uint8_t CVT5 = 5;
static constexpr uint8_t CVT6 = 6;
static constexpr uint8_t CVT7 = 7;
static constexpr uint8_t CVT8 = 8;
static constexpr uint8_t CVT9 = 9;
static constexpr uint8_t CVT10 = 10;
static constexpr uint8_t CVT11 = 11;
static constexpr uint8_t CVT12 = 12;

static constexpr uint8_t BANDOFF = 250;
static constexpr uint8_t BTOF1 = 251;
static constexpr uint8_t BTOF2 = 252;
static constexpr uint8_t BTOF3 = 253;
static constexpr uint8_t BTOF4 = 254;
static constexpr uint8_t BTOF5 = 255;
static constexpr uint8_t BVETO = 256;

}  // namespace Core