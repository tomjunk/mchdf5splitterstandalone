#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>

#include "FDHDChannelMapSP.h"
#include "Fragment.hpp"
#include "WIB2Frame.hpp"

int main(int argc, char **argv)
{

  dune::FDHDChannelMapSP chanmap;
  chanmap.ReadMapFromFiles("FDHDChannelMap_v1_wireends.txt","FDHD_CrateMap_v1.txt");
  std::string infilename = "../datfiles/TriggerRecord00001_0000TPCAPA139.dat";
  FILE *infile = fopen(infilename.data(),"r");

  std::vector<char> infiledata;

  while (1)
    {
      char c=fgetc(infile);
      if (feof(infile)) break;
      infiledata.push_back(c);
    }
  fclose(infile);

  // assume there are ten links worth of data.  Each one has a fragment in it and they
  // have simply been concatenated.  This is what mcsplit.sh does (arbitrary)
  
  std::cout << "Input file size: " << infiledata.size() << std::endl;
  if ( infiledata.size() % 10 != 0)
    {
      std::cout << "Input file size not divisible by 10. Stopping." << std::endl;
      return 1;
    }
  size_t fragsize = infiledata.size() / 10;

  for (size_t link = 0; link < 9; ++link)
    {
      size_t ibegin = link*fragsize;
      dunedaq::daqdataformats::Fragment frag( &infiledata[ibegin], dunedaq::daqdataformats::Fragment::BufferAdoptionMode::kReadOnlyMode);
      size_t n_frames = (fragsize - sizeof(dunedaq::daqdataformats::FragmentHeader))/sizeof(dunedaq::detdataformats::wib2::WIB2Frame);
      std::cout << "n_frames calc: " << fragsize << " " << sizeof(dunedaq::daqdataformats::FragmentHeader) << " " << sizeof(dunedaq::detdataformats::wib2::WIB2Frame) << " " << n_frames << std::endl;
      std::vector<std::vector<short> > adc_vectors(256);
      unsigned int slot = 0, link_from_frameheader = 0, crate = 0;

      for (size_t i = 0; i < n_frames; ++i)
	{
	  // dump WIB frames in hex
	  //std::cout << "Frame number: " << i << std::endl;
	  // size_t wfs32 = sizeof(dunedaq::detdataformats::wib2::WIB2Frame)/4;
	  //uint32_t *fdp = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(frag.get_data()) + i*sizeof(dunedaq::detdataformats::wib2::WIB2Frame));
	  //std::cout << std::dec;
	  //for (size_t iwdt = 0; iwdt < 1; iwdt++)  // dumps just the first 32 bits.  use wfs32 if you want them all
	  //{
	  //  std::cout << iwdt << " : 10987654321098765432109876543210" << std::endl;
	  //  std::cout << iwdt << " : " << std::bitset<32>{fdp[iwdt]} << std::endl;
	  //}
	  //std::cout << std::dec;

	  auto frame = reinterpret_cast<dunedaq::detdataformats::wib2::WIB2Frame*>(static_cast<uint8_t*>(frag.get_data()) + i*sizeof(dunedaq::detdataformats::wib2::WIB2Frame));
	  for (size_t j = 0; j < adc_vectors.size(); ++j)
	    {
	      adc_vectors[j].push_back(frame->get_adc(j));
	    }
              
	  if (i == 0)
	    {
	      crate = frame->header.crate;
	      slot = frame->header.slot;
	      link_from_frameheader = frame->header.link;
	    }
	}
	  std::cout << " crate, slot, link(HDF5 group), link(WIB Header): "  << crate << ", " << slot << ", " << link << ", " << link_from_frameheader << std::endl;

      for (size_t iChan = 0; iChan < 256; ++iChan)
	{
	  const std::vector<short> & v_adc = adc_vectors[iChan];

	  uint32_t slotloc = slot;
	  slotloc &= 0x7;

	  auto hdchaninfo = chanmap.GetChanInfoFromWIBElements (crate, slotloc, link_from_frameheader, iChan); 
	  unsigned int offline_chan = hdchaninfo.offlchan;

	  std::cout << "Channel: " << offline_chan << " nsamples: " << v_adc.size() << std::endl;
	  // v_adc contains the waveform for this channel
	}
    }
  
  return 0;
}
