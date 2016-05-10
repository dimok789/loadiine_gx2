/****************************************************************************
 * Copyright (C) 2015 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <malloc.h>
#include <string.h>
#include <gctypes.h>
#include "common/common.h"
#include "memory_area_table.h"

typedef struct _memory_values_t
{
    unsigned int start_address;
    unsigned int end_address;
} memory_values_t;

static const memory_values_t mem_vals_400[] =
{
    { 0x2E573BFC, 0x2FF8F83C }, // 26735 kB
    { 0x2D86D318, 0x2DFFFFFC }, // 7755 kB
    { 0x2CE59830, 0x2D3794D8 }, // 5247 kB
    { 0x2D3795AC, 0x2D854300 }, // 4971 kB
    { 0x28FEC800, 0x293B29D0 }, // 3864 kB
    { 0x29BC200C, 0x29D79B94 }, // 1758 kB
    { 0x2A517A68, 0x2A6794B8 }, // 1414 kB
    { 0x288C1D80, 0x28A69FA0 }, // 1696 kB

    { 0, 0 }
};

static const memory_values_t mem_vals_410[] =
{
//        { 0x28041760, 0x28049D0C } // 33 kB
//        { 0x280608F4, 0x2806C97C } // 48 kB
//        { 0x280953C8, 0x280A1324 } // 47 kB
//        { 0x280A1358, 0x280AD388 } // 48 kB
//        { 0x280C9040, 0x280D0ABC } // 30 kB
//        { 0x280D0AD8, 0x28113FBC } // 269 kB
//        { 0x2812575C, 0x2817A53C } // 339 kB
//        { 0x2817A6A0, 0x281BA53C } // 255 kB
//        { 0x281D571C, 0x2820253C } // 179 kB
//        { 0x28234D00, 0x2824B33C } // 89 kB
//        { 0x2824E300, 0x2828D7BC } // 253 kB
//        { 0x282A8DF0, 0x282B63FC } // 53 kB
//        { 0x282BC524, 0x282C62FC } // 39 kB
//        { 0x2835A988, 0x28366804 } // 47 kB
//        { 0x2836E05C, 0x28378DBC } // 43 kB
//        { 0x283A735C, 0x284D2A64 } // 1197 kB (1 MB)
//        { 0x284D76B0, 0x285021FC } // 170 kB
//        { 0x285766A4, 0x28583E4C } // 53 kB
//        { 0x28590E5C, 0x2859B248 } // 40 kB
//        { 0x2859B288, 0x285AE06C } // 75 kB
//        { 0x285B7108, 0x285C0A7C } // 38 kB
//        { 0x285C38A0, 0x285D089C } // 52 kB
//        { 0x285D0A84, 0x285DC63C } // 46 kB
//        { 0x285E0A84, 0x285F089C } // 63 kB
//        { 0x285F7FD0, 0x286037D8 } // 46 kB
//        { 0x2860E3E4, 0x28621B00 } // 77 kB
//        { 0x286287B0, 0x28638BC0 } // 65 kB
//        { 0x2863F4A0, 0x2864DE00 } // 58 kB
//        { 0x2864F1FC, 0x28656EE0 } // 31 kB
//        { 0x2865AF44, 0x286635A0 } // 33 kB
//        { 0x2866F774, 0x2867C680 } // 51 kB
//        { 0x2867FAC0, 0x286A2CA0 } // 140 kB
//        { 0x286B3540, 0x286C1900 } // 56 kB
//        { 0x286C64A4, 0x286DDB80 } // 93 kB
//        { 0x286E640C, 0x286F1DC0 } // 46 kB
//        { 0x286F3884, 0x2870D3C0 } // 102 kB
//        { 0x28710824, 0x28719D80 } // 37 kB
//        { 0x2872A674, 0x2873B180 } // 66 kB
//        { 0x287402F0, 0x28758780 } // 97 kB
//        { 0x287652F0, 0x28771C00 } // 50 kB
//        { 0x287F878C, 0x2880A680 } // 71 kB
//        { 0x2880F4AC, 0x2881E6E0 } // 60 kB
//        { 0x28821488, 0x28829A40 } // 33 kB
//        { 0x2882A5D0, 0x288385BC } // 55 kB
//        { 0x288385D8, 0x28854780 } // 112 kB
//        { 0x28857984, 0x28864F80 } // 53 kB
//        { 0x28870AC0, 0x2887CAC0 } // 48 kB
//        { 0x2887CAC8, 0x28888CC8 } // 48 kB
//        { 0x28888CD0, 0x28894ED0 } // 48 kB
//        { 0x28894ED8, 0x288BE0DC } // 164 kB
//        { 0x288C1C70, 0x28AD9ED4 } // 2144 kB (2 MB)
//        { 0x28AD9F04, 0x28B66100 } // 560 kB
//        { 0x28B748A8, 0x28B952E0 } // 130 kB
//        { 0x28B9AB58, 0x28BA2480 } // 30 kB
//        { 0x28BA3D00, 0x28BC21C0 } // 121 kB
//        { 0x28BC2F08, 0x28BD9860 } // 90 kB
//        { 0x28BED09C, 0x28BFDD00 } // 67 kB
//        { 0x28C068F0, 0x28C2E220 } // 158 kB
//        { 0x28CC4C6C, 0x28CF6834 } // 198 kB
//        { 0x28D3DD64, 0x28D4BF8C } // 56 kB
//        { 0x28D83C4C, 0x28DD0284 } // 305 kB
//        { 0x28DDDED4, 0x28E84294 } // 664 kB
//        { 0x28E99C7C, 0x28F382A4 } // 633 kB
//        { 0x28F45EF4, 0x28FEC2B4 } // 664 kB
//        { 0x28FEC800, 0x293B2A18 } // 3864 kB (3 MB)
//        { 0x293E187C, 0x293EC7FC } // 43 kB
//        { 0x295C7240, 0x295D523C } // 56 kB
//        { 0x295DA8DC, 0x295E323C } // 34 kB
//        { 0x295ED6C0, 0x295F6FDC } // 38 kB
//        { 0x29606340, 0x2960FC5C } // 38 kB
//        { 0x2964F040, 0x29657C3C } // 35 kB
//        { 0x296E0EBC, 0x296EBDBC } // 43 kB
//        { 0x2998DFB4, 0x2999DEE4 } // 63 kB
//        { 0x2999E6A8, 0x299BE9C4 } // 128 kB
//        { 0x29B8DF40, 0x29BA09DC } // 74 kB
//        { 0x29BC200C, 0x29D79B94 } // 1758 kB (1 MB)
//        { 0x29DA9694, 0x29DB1694 } // 32 kB
//        { 0x2A3D7558, 0x2A427558 } // 320 kB
//        { 0x2A42769C, 0x2A47769C } // 320 kB
//        { 0x2A4777E0, 0x2A4C77E0 } // 320 kB
//        { 0x2A4C7924, 0x2A517924 } // 320 kB
//        { 0x2A517A68, 0x2A6794B8 } // 1414 kB (1 MB)
//        { 0x2AD17528, 0x2AD4EA24 } // 221 kB
//        { 0x2B038C4C, 0x2B1794C8 } // 1282 kB (1 MB)
//        { 0x2BBA990C, 0x2BBB983C } // 63 kB
//        { 0x2BBBA160, 0x2BC82164 } // 800 kB
//        { 0x2BD0000C, 0x2BD71638 } // 453 kB
//        { 0x2BD7170C, 0x2BD83B0C } // 73 kB
//        { 0x2BDBA000, 0x2BDCA028 } // 64 kB
//        { 0x2BDCE000, 0x2BDDE028 } // 64 kB
//        { 0x2BDE2E34, 0x2BDF2D64 } // 63 kB
//        { 0x2BDF35E8, 0x2BE031BC } // 62 kB
//        { 0x2BE052A4, 0x2BE151D4 } // 63 kB
//        { 0x2BE174AC, 0x2BE27244 } // 63 kB
//        { 0x2BE3AC80, 0x2BE48C80 } // 56 kB
//        { 0x2BE49EDC, 0x2BE56C7C } // 51 kB
//        { 0x2BE82F70, 0x2BE92E9C } // 63 kB
//        { 0x2BE9ADBC, 0x2BEA8DBC } // 56 kB
//        { 0x2BEAAB7C, 0x2BEB6DBC } // 48 kB
//        { 0x2BEC0F3C, 0x2BECEF3C } // 56 kB
//        { 0x2BED45DC, 0x2BEDCF3C } // 34 kB
//        { 0x2BEE73C0, 0x2BEF0CDC } // 38 kB
//        { 0x2BF00040, 0x2BF0995C } // 38 kB
//        { 0x2BF48D40, 0x2BF5193C } // 35 kB
//        { 0x2BFDABBC, 0x2BFE5ABC } // 43 kB
//        { 0x2C03DA40, 0x2C045D7C } // 32 kB
//        { 0x2C179450, 0x2C18937C } // 63 kB
//        { 0x2C1DC940, 0x2C1EA93C } // 56 kB
//        { 0x2C1EABDC, 0x2C1F893C } // 55 kB
//        { 0x2C239A80, 0x2C243D3C } // 40 kB
//        { 0x2CE10224, 0x2CE3683C } // 153 kB
//        { 0x2CE374F4, 0x2CE473A4 } // 63 kB
//        { 0x2CE49830, 0x2D3794D8 } // 5311 kB (5 MB)
//        { 0x2D3795AC, 0x2D854300 } // 4971 kB (4 MB)
//        { 0x2D8546B0, 0x2D8602C4 } // 47 kB
//        { 0x2D86D318, 0x2DFFFFFC } // 7755 kB (7 MB)
//        { 0x2E2DCD60, 0x2E2E4D7C } // 32 kB
//        { 0x2E33F160, 0x2E365AFC } // 154 kB
//        { 0x2E37AC40, 0x2E39BB3C } // 131 kB
//        { 0x2E3A6EF0, 0x2E3CA2FC } // 141 kB
//        { 0x2E3D9EE0, 0x2E400B3C } // 155 kB
//        { 0x2E43A8F0, 0x2E442BBC } // 32 kB
//        { 0x2E46EC90, 0x2E48E27C } // 125 kB
//        { 0x2E497F90, 0x2E4A147C } // 37 kB
//        { 0x2E4A5B40, 0x2E4C67BC } // 131 kB
//        { 0x2E4FBEF0, 0x2E52697C } // 170 kB
//        { 0x2E550750, 0x2E57333C } // 138 kB
//        { 0x2E573F3C, 0x2FF8F07C } // 226732 kB (26 MB)
//        { 0x31000000, 0x31E1FFFC } // 614464 kB (14 MB)
//        { 0x320A5D80, 0x320AEA3C } // 35 kB
//        { 0x320E8670, 0x3210017C } // 94 kB
//        { 0x3212609C, 0x3213187C } // 45 kB
//        { 0x3219DF08, 0x321B72BC } // 100 kB
//        { 0x3300ED34, 0x3301AD3C } // 48 kB
//        { 0x33041760, 0x33049D0C } // 33 kB
//        { 0x330608F8, 0x3306C97C } // 48 kB
//        { 0x33089D80, 0x33095284 } // 45 kB
//        { 0x33095470, 0x330A1324 } // 47 kB
//        { 0x330A1358, 0x330ADC10 } // 50 kB
//        { 0x330C9040, 0x330D0ABC } // 30 kB
//        { 0x330D0AD8, 0x3311F9CC } // 315 kB
//        { 0x3312575C, 0x3320A63C } // 915 kB
//        { 0x33234D00, 0x3324B33C } // 89 kB
//        { 0x3324E300, 0x3328D7BC } // 253 kB
//        { 0x3329D134, 0x332CA324 } // 180 kB
//        { 0x3332B200, 0x33340C88 } // 86 kB
//        { 0x3335A440, 0x335021FC } // 1695 kB (1 MB)
//        { 0x3350A778, 0x3391680C } // 4144 kB (4 MB)
//        { 0x3391A444, 0x3392A25C } // 63 kB
//        { 0x3392A444, 0x33939EB4 } // 62 kB
//        { 0x3393A444, 0x3394A25C } // 63 kB
//        { 0x339587C0, 0x33976C80 } // 121 kB
//        { 0x339779C8, 0x3398E320 } // 90 kB
//        { 0x3399AE74, 0x339A7D80 } // 51 kB
//        { 0x339AB1C0, 0x339CE3A0 } // 140 kB
//        { 0x339CEB28, 0x339DEC38 } // 64 kB
//        { 0x339DEC40, 0x339ED000 } // 56 kB
//        { 0x339F1BA4, 0x33A09280 } // 93 kB
//        { 0x33A0C6E4, 0x33A15C40 } // 37 kB
//        { 0x33A15D64, 0x33EBFFFC } // 4776 kB (4 MB)
//        { 0x33F01380, 0x33F21FFC } // 131 kB
//        { 0x33F44820, 0x33F6B1BC } // 154 kB
//        { 0x33F80300, 0x33FA11FC } // 131 kB
//        { 0x33FA4D3C, 0x33FEDAFC } // 291 kB
//        { 0x33FFFFD4, 0x38FFFFFC } // 81920 kB (80 MB)
    {0, 0}
};

static const memory_values_t mem_vals_500[] =
{
    { 0x2E605CBC, 0x2FF849BC }, // size 26733828 (26107 kB) (25 MB)
    { 0x2CAE7878, 0x2D207DB4 }, // size 7472448 (7297 kB) (7 MB)
    { 0x2D3B966C, 0x2D8943C0 }, // size 5090648 (4971 kB) (4 MB)
    { 0x2D8AD3D8, 0x2DFFFFFC }, // size 7679016 (7499 kB) (7 MB)
    // TODO: Check which of those areas are usable
//		{ 0x283A73DC, 0x284D2AE4 } // size 1226508 (1197 kB) (1 MB)
//		{ 0x29030800, 0x293F69FC } // size 3957248 (3864 kB) (3 MB)
//		{ 0x2970200C, 0x298B9C54 } // size 1801292 (1759 kB) (1 MB)
//		{ 0x2A057B68, 0x2A1B9578 } // size 1448468 (1414 kB) (1 MB)

//		{ 0x29030800, 0x293F69FC } // size 3957248 (3864 kB) (3 MB)
//		{ 0x2970200C, 0x298B9C54 } // size 1801292 (1759 kB) (1 MB)
//		{ 0x2A057B68, 0x2A1B9578 } // size 1448468 (1414 kB) (1 MB)
//		{ 0x288EEC30, 0x28B06E94 } // size 2196072 (2144 kB) (2 MB)
//		{ 0x283A73DC, 0x284D2AE4 } // size 1226508 (1197 kB) (1 MB)
//		{ 0x3335A4C0, 0x335021FC } // size 1736000 (1695 kB) (1 MB)
//		{ 0x3350C1D4, 0x339182CC } // size 4243708 (4144 kB) (4 MB)
//		{ 0x33A14094, 0x33EBFFFC } // size 4898668 (4783 kB) (4 MB)
//		{ 0x33FFFFD4, 0x38FFFFFC } // size 83886124 (81920 kB) (80 MB)
    {0, 0}
};

static const memory_values_t mem_vals_532[] =
{
    // TODO: Check which of those areas are usable
//        {0x28000000 + 0x000DCC9C, 0x28000000 + 0x00174F80}, // 608 kB
//        {0x28000000 + 0x00180B60, 0x28000000 + 0x001C0A00}, // 255 kB
//        {0x28000000 + 0x001ECE9C, 0x28000000 + 0x00208CC0}, // 111 kB
//        {0x28000000 + 0x00234180, 0x28000000 + 0x0024B444}, // 92 kB
//        {0x28000000 + 0x0024D8C0, 0x28000000 + 0x0028D884}, // 255 kB
//        {0x28000000 + 0x003A745C, 0x28000000 + 0x004D2B68}, // 1197 kB
//        {0x28000000 + 0x004D77B0, 0x28000000 + 0x00502200}, // 170 kB
//        {0x28000000 + 0x005B3A88, 0x28000000 + 0x005C6870}, // 75 kB
//        {0x28000000 + 0x0061F3E4, 0x28000000 + 0x00632B04}, // 77 kB
//        {0x28000000 + 0x00639790, 0x28000000 + 0x00649BC4}, // 65 kB
//        {0x28000000 + 0x00691490, 0x28000000 + 0x006B3CA4}, // 138 kB
//        {0x28000000 + 0x006D7BCC, 0x28000000 + 0x006EEB84}, // 91 kB
//        {0x28000000 + 0x00704E44, 0x28000000 + 0x0071E3C4}, // 101 kB
//        {0x28000000 + 0x0073B684, 0x28000000 + 0x0074C184}, // 66 kB
//        {0x28000000 + 0x00751354, 0x28000000 + 0x00769784}, // 97 kB
//        {0x28000000 + 0x008627DC, 0x28000000 + 0x00872904}, // 64 kB
//        {0x28000000 + 0x008C1E98, 0x28000000 + 0x008EB0A0}, // 164 kB
//        {0x28000000 + 0x008EEC30, 0x28000000 + 0x00B06E98}, // 2144 kB
//        {0x28000000 + 0x00B06EC4, 0x28000000 + 0x00B930C4}, // 560 kB
//        {0x28000000 + 0x00BA1868, 0x28000000 + 0x00BC22A4}, // 130 kB
//        {0x28000000 + 0x00BC48F8, 0x28000000 + 0x00BDEC84}, // 104 kB
//        {0x28000000 + 0x00BE3DC0, 0x28000000 + 0x00C02284}, // 121 kB
//        {0x28000000 + 0x00C02FC8, 0x28000000 + 0x00C19924}, // 90 kB
//        {0x28000000 + 0x00C2D35C, 0x28000000 + 0x00C3DDC4}, // 66 kB
//        {0x28000000 + 0x00C48654, 0x28000000 + 0x00C6E2E4}, // 151 kB
//        {0x28000000 + 0x00D04E04, 0x28000000 + 0x00D36938}, // 198 kB
//        {0x28000000 + 0x00DC88AC, 0x28000000 + 0x00E14288}, // 302 kB
//        {0x28000000 + 0x00E21ED4, 0x28000000 + 0x00EC8298}, // 664 kB
//        {0x28000000 + 0x00EDDC7C, 0x28000000 + 0x00F7C2A8}, // 633 kB
//        {0x28000000 + 0x00F89EF4, 0x28000000 + 0x010302B8}, // 664 kB
//        {0x28000000 + 0x01030800, 0x28000000 + 0x013F69A0}, // 3864 kB
//        {0x28000000 + 0x016CE000, 0x28000000 + 0x016E0AA0}, // 74 kB
//        {0x28000000 + 0x0170200C, 0x28000000 + 0x018B9C58}, // 1759 kB
//        {0x28000000 + 0x01F17658, 0x28000000 + 0x01F6765C}, // 320 kB
//        {0x28000000 + 0x01F6779C, 0x28000000 + 0x01FB77A0}, // 320 kB
//        {0x28000000 + 0x01FB78E0, 0x28000000 + 0x020078E4}, // 320 kB
//        {0x28000000 + 0x02007A24, 0x28000000 + 0x02057A28}, // 320 kB
//        {0x28000000 + 0x02057B68, 0x28000000 + 0x021B957C}, // 1414 kB
//        {0x28000000 + 0x02891528, 0x28000000 + 0x028C8A28}, // 221 kB
//        {0x28000000 + 0x02BBCC4C, 0x28000000 + 0x02CB958C}, // 1010 kB
//        {0x28000000 + 0x0378D45C, 0x28000000 + 0x03855464}, // 800 kB
//        {0x28000000 + 0x0387800C, 0x28000000 + 0x03944938}, // 818 kB
//        {0x28000000 + 0x03944A08, 0x28000000 + 0x03956E0C}, // 73 kB
//        {0x28000000 + 0x04A944A4, 0x28000000 + 0x04ABAAC0}, // 153 kB
//        {0x28000000 + 0x04ADE370, 0x28000000 + 0x0520EAB8}, // 7361 kB      // ok
//        {0x28000000 + 0x053B966C, 0x28000000 + 0x058943C4}, // 4971 kB      // ok
//        {0x28000000 + 0x058AD3D8, 0x28000000 + 0x06000000}, // 7499 kB
//        {0x28000000 + 0x0638D320, 0x28000000 + 0x063B0280}, // 139 kB
//        {0x28000000 + 0x063C39E0, 0x28000000 + 0x063E62C0}, // 138 kB
//        {0x28000000 + 0x063F52A0, 0x28000000 + 0x06414A80}, // 125 kB
//        {0x28000000 + 0x06422810, 0x28000000 + 0x0644B2C0}, // 162 kB
//        {0x28000000 + 0x064E48D0, 0x28000000 + 0x06503EC0}, // 125 kB
//        {0x28000000 + 0x0650E360, 0x28000000 + 0x06537080}, // 163 kB
//        {0x28000000 + 0x0653A460, 0x28000000 + 0x0655C300}, // 135 kB
//        {0x28000000 + 0x0658AA40, 0x28000000 + 0x065BC4C0}, // 198 kB       // ok
//        {0x28000000 + 0x065E51A0, 0x28000000 + 0x06608E80}, // 143 kB       // ok
//        {0x28000000 + 0x06609ABC, 0x28000000 + 0x07F82C00}, // 26084 kB     // ok

//        {0x30000000 + 0x000DCC9C, 0x30000000 + 0x00180A00}, // 655 kB
//        {0x30000000 + 0x00180B60, 0x30000000 + 0x001C0A00}, // 255 kB
//        {0x30000000 + 0x001F5EF0, 0x30000000 + 0x00208CC0}, // 75 kB
//        {0x30000000 + 0x00234180, 0x30000000 + 0x0024B444}, // 92 kB
//        {0x30000000 + 0x0024D8C0, 0x30000000 + 0x0028D884}, // 255 kB
//        {0x30000000 + 0x003A745C, 0x30000000 + 0x004D2B68}, // 1197 kB
//        {0x30000000 + 0x006D3334, 0x30000000 + 0x00772204}, // 635 kB
//        {0x30000000 + 0x00789C60, 0x30000000 + 0x007C6000}, // 240 kB
//        {0x30000000 + 0x00800000, 0x30000000 + 0x01E20000}, // 22876 kB     // ok
    { 0x2E609ABC, 0x2FF82C00 }, // 26084 kB
    { 0x29030800, 0x293F69A0 }, // 3864 kB
    { 0x288EEC30, 0x28B06E98 }, // 2144 kB
    { 0x2D3B966C, 0x2D8943C4 }, // 4971 kB
    { 0x2CAE0370, 0x2D20EAB8 }, // 7361 kB
    { 0x2D8AD3D8, 0x2E000000 }, // 7499 kB

    {0, 0}
}; // total : 66mB + 25mB

static const memory_values_t mem_vals_540[] =
{
    { 0x2E609EFC, 0x2FF82000 }, // 26083 kB
    { 0x29030800, 0x293F6000 }, // 3864 kB
    { 0x288EEC30, 0x28B06800 }, // 2144 kB
    { 0x2D3B966C, 0x2D894000 }, // 4971 kB
    { 0x2CB56370, 0x2D1EF000 }, // 6756 kB
    { 0x2D8AD3D8, 0x2E000000 }, // 7499 kB
    { 0x2970200C, 0x298B9800 }, // 1759 kB
    { 0x2A057B68, 0x2A1B9000 }, // 1414 kB
    { 0x2ABBCC4C, 0x2ACB9000 }, // 1010 kB
    {0, 0}
};

//! retain container for our memory area table data
static unsigned char ucMemAreaTableBuffer[0xff];

s_mem_area * memoryGetAreaTable(void)
{
    return (s_mem_area *) (ucMemAreaTableBuffer);
}

static inline void memoryAddArea(int start, int end, int cur_index)
{
    // Create and copy new memory area
    s_mem_area * mem_area = memoryGetAreaTable();
    mem_area[cur_index].address = start;
    mem_area[cur_index].size    = end - start;
    mem_area[cur_index].next    = 0;

    // Fill pointer to this area in the previous area
    if (cur_index > 0)
    {
        mem_area[cur_index - 1].next = &mem_area[cur_index];
    }
}

/* Create memory areas arrays */
void memoryInitAreaTable()
{
    u32 ApplicationMemoryEnd;

    asm volatile("lis %0, __CODE_END@h; ori %0, %0, __CODE_END@l" : "=r" (ApplicationMemoryEnd));

    // This one seems to be available on every firmware and therefore its our code area but also our main RPX area behind our code
    // 22876 kB - our application    // ok
    if(OS_FIRMWARE <= 400) {
        memoryAddArea(ApplicationMemoryEnd + 0x4B000000, 0x4B000000 + 0x01E20000, 0);
    }
    else {
        memoryAddArea(ApplicationMemoryEnd + 0x30000000, 0x30000000 + 0x01E20000, 0);
    }

    const memory_values_t * mem_vals = NULL;

    switch(OS_FIRMWARE)
    {
    case 400: {
        mem_vals = mem_vals_400;
        break;
    }
    case 500: {
        mem_vals = mem_vals_500;
        break;
    }
    case 532: {
        mem_vals = mem_vals_532;
        break;
    }
    case 540:
	case 550: {
        mem_vals = mem_vals_540;
        break;
    }
    default:
        return; // no known values
    }

    // Fill entries
    int i = 0;
    while (mem_vals[i].start_address)
    {
        memoryAddArea(mem_vals[i].start_address, mem_vals[i].end_address, i + 1);
        i++;
    }
}
