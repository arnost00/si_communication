#pragma once

#include <boost/type_traits.hpp>
namespace si
{
//   unsigned int crc(unsigned int uiCount,unsigned char *pucDat);

   BOOST_STATIC_CONSTANT(unsigned __int16, POLYNOM=0x8005);


   template<typename iterator> unsigned __int16 crc(std::size_t uiCount,iterator it)
   {
      short int iTmp;
      unsigned short int uiTmp,uiTmp1,uiVal;
      iterator pucTmpDat;

      if (uiCount < 2) return(0);        // response value is "0" for none or one data byte
      pucTmpDat = it;

      uiTmp1 = *pucTmpDat++;
      uiTmp1 = (uiTmp1<<8) + *pucTmpDat++;

      if (uiCount == 2) return(uiTmp1);   // response value is CRC for two data bytes
      for (iTmp=(int)(uiCount>>1);iTmp>0;iTmp--)
      {

         if (iTmp>1)
         {
            uiVal = *pucTmpDat++;
            uiVal= (uiVal<<8) + *pucTmpDat++;
         }
         else 
         {
            if (uiCount&1)               // odd number of data bytes, complete with "0"         
            {
               uiVal = *pucTmpDat;
               uiVal= (uiVal<<8);
            }
            else
            {
               uiVal=0; //letzte Werte mit 0
            }
         } 

         for (uiTmp=0;uiTmp<16;uiTmp++)
         {
            if (uiTmp1 & 0x8000)
            {
               uiTmp1  <<= 1;
               if (uiVal & 0x8000)uiTmp1++;
               uiTmp1 ^= POLYNOM;
            }
            else
            {
               uiTmp1  <<= 1;
               if (uiVal & 0x8000)uiTmp1++;
            }
            uiVal <<= 1;
         }
      }
      return(uiTmp1);
   }

}//namespace si
