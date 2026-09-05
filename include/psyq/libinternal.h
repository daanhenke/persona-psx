#ifndef LIBINTERNAL_H
#define LIBINTERNAL_H

/* Psy-Q library entry points with no published prototype.
 *
 * The linked-in libraries contain far more than the documented API: the
 * per-object helpers the signature analyser could name (`_mk_normsp`,
 * `RCpolyFT4A`), the sound driver's whole private half (`_Ss*`, `SpuVm*`),
 * libgs's ninety TMD packers, and the C runtime's internals. None of it is
 * callable from a manual, so there is no signature to write down.
 *
 * They are declared here anyway, in the old style with no argument list, for
 * one reason: tools/gen_sdk.py treats a name declared under include/psyq as
 * Psy-Q, and Psy-Q is what tools/progress.py must not count as work left to
 * do. Writing them down is also the record that each one was looked at and
 * found to be Sony's, rather than game code nobody got round to.
 *
 * Nothing includes this header. If a function here turns out to be one the
 * game calls, move it to the library header it belongs to and give it a real
 * prototype there - do not start calling it from this file.
 */

/* libgs - TMD packers and coordinate helpers */
extern void GsInitCoord2param();
extern void GsMulCoord0();
extern void GsMulCoord2();
extern void GsMulCoord3();
extern void GsSetDrawBuffClip();
extern void GsSetDrawBuffOffset();
extern void GsSortPoly();
extern void GsTMDdivF3L();
extern void GsTMDdivF3NL();
extern void GsTMDdivF4L();
extern void GsTMDdivF4LFG();
extern void GsTMDdivF4NL();
extern void GsTMDdivG3L();
extern void GsTMDdivG3NL();
extern void GsTMDdivG4L();
extern void GsTMDdivG4LFG();
extern void GsTMDdivG4NL();
extern void GsTMDdivNF3();
extern void GsTMDdivNF4();
extern void GsTMDdivNG3();
extern void GsTMDdivNG4();
extern void GsTMDdivTF3L();
extern void GsTMDdivTF3LFG();
extern void GsTMDdivTF3NL();
extern void GsTMDdivTF4L();
extern void GsTMDdivTF4LFG();
extern void GsTMDdivTF4NL();
extern void GsTMDdivTG3L();
extern void GsTMDdivTG3NL();
extern void GsTMDdivTG4L();
extern void GsTMDdivTG4LFG();
extern void GsTMDdivTG4NL();
extern void GsTMDdivTNF3();
extern void GsTMDdivTNF4();
extern void GsTMDdivTNG3();
extern void GsTMDdivTNG4();
extern void GsTMDfastF3GL();
extern void GsTMDfastF3GLFG();
extern void GsTMDfastF3GNL();
extern void GsTMDfastF3L();
extern void GsTMDfastF3LFG();
extern void GsTMDfastF3NL();
extern void GsTMDfastF4L();
extern void GsTMDfastF4LFG();
extern void GsTMDfastF4NL();
extern void GsTMDfastG3GL();
extern void GsTMDfastG3GLFG();
extern void GsTMDfastG3GNL();
extern void GsTMDfastG3L();
extern void GsTMDfastG3LFG();
extern void GsTMDfastG3NL();
extern void GsTMDfastG4L();
extern void GsTMDfastG4LFG();
extern void GsTMDfastG4NL();
extern void GsTMDfastNF3();
extern void GsTMDfastNF4();
extern void GsTMDfastNG3();
extern void GsTMDfastNG4();
extern void GsTMDfastTF3L();
extern void GsTMDfastTF3LFG();
extern void GsTMDfastTF3NL();
extern void GsTMDfastTF4L();
extern void GsTMDfastTF4LFG();
extern void GsTMDfastTF4NL();
extern void GsTMDfastTG3L();
extern void GsTMDfastTG3LFG();
extern void GsTMDfastTG3NL();
extern void GsTMDfastTG4L();
extern void GsTMDfastTG4LFG();
extern void GsTMDfastTG4NL();
extern void GsTMDfastTNF3();
extern void GsTMDfastTNF4();
extern void GsTMDfastTNG3();
extern void GsTMDfastTNG4();
extern void Gssub_make_matrix();

/* libsnd - sequencer and sound-driver internals */
extern void KeyOnCheck();
extern void SeAutoPan();
extern void SeAutoVol();
extern void SePitchBend();
extern void Snd_SetPlayMode();
extern void SsSepSetRitardando();
extern void SsSeqCalledTbyT();
extern void SsSetLoop();
extern void SsUtAllKeyOff();
extern void SsUtAutoPan();
extern void SsUtAutoVol();
extern void SsUtChangeADSR();
extern void SsUtChangePitch();
extern void SsUtGetDetVVol();
extern void SsUtGetReverbType();
extern void SsUtGetVVol();
extern void SsUtKeyOff();
extern void SsUtKeyOffV();
extern void SsUtKeyOn();
extern void SsUtPitchBend();
extern void SsUtSetDetVVol();
extern void SsUtSetReverbFeedback();
extern void SsUtVibrateOff();
extern void SsUtVibrateOn();
extern void SsVabFakeHead();
extern void SsVabOpen();
extern void SsVabOpenHeadWithMode();
extern void _SsClose();
extern void _SsContDataEntry();
extern void _SsContModulation();
extern void _SsContNrpn1();
extern void _SsContNrpn2();
extern void _SsContPortaTime();
extern void _SsContPortamento();
extern void _SsContResetAll();
extern void _SsContRpn1();
extern void _SsContRpn2();
extern void _SsGetMetaEvent();
extern void _SsGetSeqData();
extern void _SsInit();
extern void _SsInitSoundSep();
extern void _SsInitSoundSeq();
extern void _SsNoteOn();
extern void _SsReadDeltaValue();
extern void _SsSeqCalledTbyT_1per2();
extern void _SsSeqPlay();
extern void _SsSetControlChange();
extern void _SsSetPitchBend();
extern void _SsSetProgramChange();
extern void _SsSndCrescendo();
extern void _SsSndDecrescendo();
extern void _SsSndNextSep();
extern void _SsSndPause();
extern void _SsSndPlay();
extern void _SsSndReplay();
extern void _SsSndSetCres();
extern void _SsSndSetDecres();
extern void _SsSndSetPauseMode();
extern void _SsSndSetReplayMode();
extern void _SsSndSetRit();
extern void _SsSndSetVabAttr();
extern void _SsSndSetVol();
extern void _SsSndSetVolData();
extern void _SsSndStop();
extern void _SsSndTempo();
extern void _SsStart();
extern void _SsTrapIntrVSync();
extern void _SsUtBuildADSR();
extern void _SsUtResolveADSR();
extern void note2pitch();
extern void note2pitch2();

/* libspu - SPU driver and the voice manager */
extern void SetAutoPan();
extern void SetAutoVol();
extern void SpuVmAlloc();
extern void SpuVmDoAllocate();
extern void SpuVmFlush();
extern void SpuVmGetProgPan();
extern void SpuVmGetProgVol();
extern void SpuVmGetSeqLVol();
extern void SpuVmGetSeqRVol();
extern void SpuVmGetSeqVol();
extern void SpuVmInit();
extern void SpuVmKeyOff();
extern void SpuVmKeyOn();
extern void SpuVmKeyOnNow();
extern void SpuVmNoiseOff();
extern void SpuVmNoiseOn();
extern void SpuVmNoiseOnWithAdsr();
extern void SpuVmPBVoice();
extern void SpuVmPitchBend();
extern void SpuVmSeKeyOff();
extern void SpuVmSeKeyOn();
extern void SpuVmSeqKeyOff();
extern void SpuVmSetProgPan();
extern void SpuVmSetProgVol();
extern void SpuVmSetSeqVol();
extern void SpuVmSetVol();
extern void SpuVmVSetUp();
extern void _SpuDataCallback();
extern void _SpuInit();
extern void _SpuIsInAllocateArea();
extern void _SpuIsInAllocateArea_();
extern void _SpuSetAnyVoice();
extern void _spu_FgetRXXa();
extern void _spu_FiDMA();
extern void _spu_FsetRXX();
extern void _spu_FsetRXXa();
extern void _spu_gcSPU();
extern void _spu_init();
extern void _spu_r_();
extern void _spu_read();
extern void _spu_reset();
extern void _spu_setReverbAttr();
extern void _spu_t();
extern void _spu_write();
extern void _spu_writeByIO();
extern void vmNoiseOn();
extern void vmNoiseOn2();

/* libgpu - packet builders and the GPU control path */
extern void GPU_cw();
extern void GetGraphDebug();
extern void LoadClut2();
extern void MargePrim();
extern void OpenTMD();
extern void RCpolyF3();
extern void RCpolyF3A();
extern void RCpolyF4();
extern void RCpolyF4A();
extern void RCpolyFT3();
extern void RCpolyFT3A();
extern void RCpolyFT4();
extern void RCpolyFT4A();
extern void RCpolyG3();
extern void RCpolyG3A();
extern void RCpolyG4();
extern void RCpolyG4A();
extern void RCpolyGT3();
extern void RCpolyGT3A();
extern void RCpolyGT4();
extern void RCpolyGT4A();
extern void ReadTMD();
extern void SetDrawLoad();
extern void SetDrawTPage();
extern void SetPriority();
extern void _mk_normsp();
extern void _mk_spr_packet();
extern void _mk_xpndsp();
extern void checkRECT();
extern void get_ce();
extern void get_cs();
extern void get_dx();
extern void get_mode();
extern void get_ofs();
extern void get_tim_addr();
extern void get_tmd_addr();
extern void get_tw();
extern void gpu_init();

/* libgte - GTE register access and the matrix helpers */
extern void ApplyMatrixLV();
extern void ApplyRotMatrix();
extern void AverageSZ3();
extern void AverageSZ4();
extern void AverageZ3();
extern void AverageZ4();
extern void ColorCol();
extern void ColorDpq();
extern void DivideFT4();
extern void DpqColor();
extern void DpqColor3();
extern void DpqColorLight();
extern void Intpl();
extern void LightColor();
extern void LocalLight();
extern void Lzc();
extern void MulMatrix0();
extern void MulMatrix2();
extern void MulRotMatrix();
extern void MulRotMatrix0();
extern void NormalColor();
extern void NormalColor3();
extern void NormalColorCol3();
extern void NormalColorDpq();
extern void NormalColorDpq3();
extern void OuterProduct0();
extern void OuterProduct12();
extern void ReadColorMatrix();
extern void ReadLightMatrix();
extern void ReadRotMatrix();
extern void RotAverage3();
extern void RotAverageNclip3();
extern void RotAverageNclipColorCol3();
extern void RotAverageNclipColorDpq3();
extern void RotAverageNclipColorDpq3_1();
extern void RotRMD_FT4();
extern void ScaleMatrixL();
extern void SetDQA();
extern void SetDQB();
extern void SetData32();
extern void SetIR0();
extern void SetIR123();
extern void SetMAC123();
extern void SetMulMatrix();
extern void SetRGBfifo();
extern void SetRii();
extern void SetSXSYfifo();
extern void SetSZfifo3();
extern void SetSZfifo4();
extern void SetVertex0();
extern void SetVertex1();
extern void SetVertex2();
extern void SetVertexTri();
extern void Square0();
extern void Square12();
extern void TransposeMatrix();
extern void _patch_gte();
extern void csqrt();
extern void csqrt_1();
extern void gte_init();
extern void gte_read_lc();
extern void gte_rotate_z_matrix();
extern void gte_scale_matrix();
extern void gte_set_lc();
extern void print_matrix();
extern void print_vector();
extern void sin_1();

/* libcd - the CD driver below CdControl, and streaming */
extern void CD_cachefile();
extern void CD_cw();
extern void CD_datasync();
extern void CD_flush();
extern void CD_getsector();
extern void CD_init();
extern void CD_initintr();
extern void CD_initvol();
extern void CD_newmedia();
extern void CD_ready();
extern void CD_searchdir();
extern void CD_set_test_parmnum();
extern void CD_sync();
extern void CD_vol();
extern void StCdInterrupt();
extern void StCdInterrupt2();
extern void StrGetReadyFrame();
extern void _sync();
extern void cb_read();
extern void cd_read();
extern void cd_read_retry();
extern void data_ready_callback();
extern void def_cbread();
extern void def_cbready();
extern void def_cbsync();
extern void dma_execute();
extern void getintr();
extern void init_ring_status();
extern void timeout();
extern void v_wait();

/* libapi / kernel - events, the BIOS file API, exceptions */
extern void InterruptCallback();
extern void PCcreat();
extern void PCread();
extern void _SN_write();
extern void _addque();
extern void _addque2();
extern void _card_write();
extern void _clr();
extern void _ctl();
extern void _cwb();
extern void _cwc();
extern void _drs();
extern void _dws();
extern void _exeque();
extern void _getctl();
extern void _new_card();
extern void _otc();
extern void _param();
extern void _status();
extern void _version();
extern void callback();
extern void delete();
extern void get_alarm();
extern void len_param();
extern void longjmp();
extern void restartIntr();
extern void setIntr();
extern void setIntrDMA();
extern void setIntrVSync();
extern void set_alarm();
extern void setjmp();
extern void startIntr();
extern void startIntrDMA();
extern void startIntrVSync();
extern void stopIntr();
extern void trapIntr();
extern void trapIntrDMA();
extern void trapIntrVSync();

/* libetc / libpad - pads and the root counters */
extern void ChangeClearRCnt();
extern void InitPAD();
extern void PAD_dr();
extern void StartPAD();

/* libpress - the MDEC decoder */
extern void DecDCTBufSize();
extern void DecDCTGetEnv();
extern void DecDCTPutEnv();
extern void DecDCTReset();
extern void DecDCTin();
extern void DecDCTinCallback();
extern void DecDCTinSync();
extern void DecDCTout();
extern void DecDCToutCallback();
extern void DecDCToutSync();
extern void DecDCTvlc();
extern void DecDCTvlcSize();
extern void MDEC_in();
extern void MDEC_in_sync();
extern void MDEC_out();
extern void MDEC_out_sync();
extern void MDEC_reset();
extern void _make_packet();
extern void unpack_packet();

/* libc and the compiler's own runtime */
extern void __fixunsdfsi();
extern void mem2mem();
extern void memchr();
extern void memclr();
extern void prnt();
extern void scale_view_param();
extern void select_max_param();
extern void tolower();
extern void toupper();
extern void valiable_init();

/* Everything else the libraries brought in with them. */
extern void ==();

#endif
