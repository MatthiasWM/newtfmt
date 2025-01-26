
#include "package_bytes.h"

#include "tools.h"

using namespace ns;

/**
 Set the iterator back to the first byte.
 */
void PackageBytes::rewind()
{
  it_ = begin();
}

/**
 Set the iterator at a new index.
 \param[in] ix bytes from start of data
 */
void PackageBytes::seek_set(int ix)
{
  it_ = begin() + ix;
}

/**
 Return the offset of the reader within the dataset.
 \return offset in bytes
 */
int PackageBytes::tell()
{
  return (int)(it_ - begin());
}

/**
 Check if we are at the end of the dataset.
 \return true if we are at the end
 */
bool PackageBytes::eof()
{
  return it_ == end();
}

/**
 Get one byte and advance the iterator.
 \return the current byte
 */
uint8_t PackageBytes::get_ubyte() {
  return *it_++;
}

/**
 Get one 16 bit word in MSB format and advance the iterator.
 \return a short integer in the native byte order.
 */
uint16_t PackageBytes::get_ushort()
{
  uint16_t v;
  v = ((*it_++)<<8)|(*it_++);
  return v;
}

/**
 Get one 32 bit word in MSB format and advance the iterator.
 \return a integer in the native byte order.
 */
uint32_t PackageBytes::get_uint() {
  uint32_t v;
  v = ((*it_++)<<24)|((*it_++)<<16)|((*it_++)<<8)|(*it_++);
  return v;
}

/**
 Create an ASCII string and advance the iterator.
 \note This does not check if the string is actually ASCII (it may contain
 Mac specific characters with bit 7 set) and does not check if the trailing
 character is actually NUL.
 \param[in] n number of bytes in string, not counting the trailing NUL
 \param[in] trailing_nul if set (default), skip over the trailing NUL
 \return a std::string with the text, no further conversion is done.
 */
std::string PackageBytes::get_cstring(int n, bool trailing_nul) {
  PackageBytes::iterator it_start = it_;
  if (trailing_nul) {
    it_ += n+1;
    return std::string(it_start, it_-1);
  } else {
    it_ += n;
    return std::string(it_start, it_);
  }
}

/**
 Read a UTF-16 string, advance the iterator, and return the string in UTF-8.
 \note This does not check if the trailing character is actually NUL.
 \param[in] n number of 16bit shorts in string, not counting the trailing NUL.
 \param[in] trailing_nul if set (default), skip over the trailing NUL
 \return a std::string in UTF-8 format
 */
std::string PackageBytes::get_ustring(int n, bool trailing_nul) {
  std::u16string s;
  for (int i=0; i<n; i++) s += get_ushort();
  if (trailing_nul) get_ushort();
  return utf16_to_utf8(s);
}

/**
 Read a block of raw data and advance the iterator.
 \param[in] n number of bytes to read
 \return a vector with the unmodified bytes
 */
std::vector<uint8_t> PackageBytes::get_data(int n) {
  auto it_start = it_;
  it_ += n;
  return std::vector<uint8_t>(it_start, it_);
}

/**
 Update the index to be aligned to a.
 \param[in] a alignment must be power of two (usually 4 or 8)
 */
void PackageBytes::align(int a)
{
  int p = tell();
  int p_aligned = (p+a-1) & ~(a-1);
  seek_set(p_aligned);
}

