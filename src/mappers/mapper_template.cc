#include "mappers/mapper_template.hh"

Mapper_Template::Mapper_Template(uint8_t banks_PRG, uint8_t banks_CHR) {
  this->banks_CHR = banks_CHR;
  this->banks_PRG = banks_PRG;
}

Mapper_Template::~Mapper_Template() {}
