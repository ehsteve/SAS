#include <sys/time.h>

#include <fstream>
#include <iostream>

#include "Telemetry.hpp"

#define INDEX_TELEMETRY_TYPE 2
#define INDEX_SOURCE_ID 3
#define INDEX_PAYLOAD_LENGTH 4
#define INDEX_CHECKSUM 6
#define INDEX_NANOSECONDS 8
#define INDEX_SECONDS 12
#define INDEX_PAYLOAD 16

using std::ostream;

class TelemetryPacketSizeException : public std::exception
{
  virtual const char* what() const throw()
  {
    return "TelemetryPacket payload is too large";
  }
} tpSizeException;

class TelemetryPacketQueueEmptyException : public std::exception
{
  virtual const char* what() const throw()
  {
    return "TelemetryPacketQueue has no more telemetry packets";
  }
} tpqEmptyException;

TelemetryPacket::TelemetryPacket(uint8_t typeID, uint8_t sourceID)
{
  //Zeros are payload length and checksum
  *this << typeID << sourceID << (uint16_t)0 << (uint16_t)0;
  //Zeros are nanoseconds and seconds
  *this << (uint32_t)0 << (uint32_t)0;
  setReadIndex(INDEX_PAYLOAD);
}

TelemetryPacket::TelemetryPacket(const uint8_t *ptr, uint16_t num)
  : Packet(ptr, num)
{
  setReadIndex(INDEX_PAYLOAD);
}

void TelemetryPacket::finish()
{
  writePayloadLength();
  writeTime();
  writeChecksum();
}

void TelemetryPacket::writePayloadLength()
{
  if(getLength() > TELEMETRY_PACKET_MAX_SIZE) throw tpSizeException;
  replace(INDEX_PAYLOAD_LENGTH, (uint16_t)(getLength()-INDEX_PAYLOAD));
}

void TelemetryPacket::writeChecksum()
{
  replace(INDEX_CHECKSUM, (uint16_t)0);
  replace(INDEX_CHECKSUM, (uint16_t)checksum());
}

void TelemetryPacket::writeTime()
{
  timeval now;
  gettimeofday(&now, NULL);
  replace(INDEX_NANOSECONDS, (uint32_t)now.tv_usec*1000);
  replace(INDEX_SECONDS, (uint32_t)now.tv_sec);
}

bool TelemetryPacket::valid()
{
  return Packet::valid();
}

uint8_t TelemetryPacket::getTypeID()
{
  uint8_t value;
  this->readAtTo(INDEX_TELEMETRY_TYPE, value);
  return value;
}

uint8_t TelemetryPacket::getSourceID()
{
  uint8_t value;
  this->readAtTo(INDEX_SOURCE_ID, value);
  return value;
}

TelemetryPacketQueue &operator<<(TelemetryPacketQueue &tpq, const TelemetryPacket &tp)
{
  tpq.push_back(tp);
  return tpq;
}

TelemetryPacketQueue &operator<<(TelemetryPacketQueue &tpq, TelemetryPacketQueue &other)
{
  tpq.splice(tpq.end(), other);
  return tpq;
}

TelemetryPacketQueue &operator>>(TelemetryPacketQueue &tpq, TelemetryPacket &tp)
{
  if(tpq.empty()) throw tpqEmptyException;
  tp = tpq.front();
  tpq.pop_front();
  return tpq;
}

ostream &operator<<(ostream &os, TelemetryPacketQueue &tpq)
{
  if(tpq.empty()) throw tpqEmptyException;
  int i = 0;
  for (TelemetryPacketQueue::iterator it=tpq.begin(); it != tpq.end(); ++it) {
    os << ++i << ": "<< *it << std::endl;
  }
  return os;
}

TelemetryPacketQueue::TelemetryPacketQueue() : filter_typeID(false), filter_sourceID(false)
{
}

void TelemetryPacketQueue::filterTypeID(uint8_t typeID)
{
  filter_typeID = true;
  i_typeID = typeID;
}

void TelemetryPacketQueue::filterSourceID(uint8_t sourceID)
{
  filter_sourceID = true;
  i_sourceID = sourceID;
}

void TelemetryPacketQueue::resetFilters()
{
  filter_typeID = false;
  filter_sourceID = false;
}

void TelemetryPacketQueue::add_file(const char* file)
{
  uint32_t ct_sync = 0, ct_length = 0, ct_valid = 0;
  uint32_t ct_typeID = 0, ct_sourceID = 0;
  std::streampos cur;

  bool pass_sourceID, pass_typeID;

  uint8_t buffer[TELEMETRY_PACKET_MAX_SIZE];
  buffer[0] = 0x9a;

  uint16_t length;

  TelemetryPacket tp((uint8_t)0x0, (uint8_t)0x0);

  std::ifstream ifs(file);

  while (!ifs.eof()) {

    if(ifs.get() == 0x9a) {
      if(ifs.peek() == 0xc3) {
        ct_sync++; // sync word found

        cur = ifs.tellg(); // points one byte into sync word
	ifs.seekg(3, std::ios::cur);
        ifs.read((char *)&length, 2);

	if(length > TELEMETRY_PACKET_MAX_SIZE-16) continue; //invalid payload size
	ct_length++;

        ifs.seekg(cur);

        ifs.read((char *)buffer+1, length+15);

	tp = TelemetryPacket(buffer, length+16);

        if(tp.valid()) {
          ct_valid++;
          pass_sourceID = !(filter_sourceID && !(tp.getSourceID() == i_sourceID));
          pass_typeID = !(filter_typeID && !(tp.getTypeID() == i_typeID));
	  if(pass_sourceID) ct_sourceID++;
	  if(pass_typeID) ct_typeID++;
          if(pass_sourceID && pass_typeID) *this << tp;
        }

        ifs.seekg(cur);
      }
    }

  }

  std::cout << ct_sync << " sync words found, ";
  std::cout << ct_valid << " packets with valid checksums\n";

  if(filter_sourceID) {
    std::cout << ct_sourceID << " packets with with the filtered source ID\n";
  }
  if(filter_typeID) {
    std::cout << ct_typeID << " packets with with the filtered type ID\n";
  }

}
