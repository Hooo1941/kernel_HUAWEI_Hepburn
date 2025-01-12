#!/bin/bash
#######################################################################
##
## description : generate config.h and config.mak of lib_k3_ffmpeg.so
#######################################################################

FF_CONFIG_OPTIONS="
    --disable-stripping
    --disable-asm 
    --disable-programs
    --disable-doc
    --disable-debug
    --disable-avdevice
    --disable-avfilter
    --disable-avresample
    --disable-postproc
    --disable-bsfs
    --disable-iconv
    --disable-xlib
    --disable-zlib
    --disable-cuvid
    --disable-cuda
    --disable-libxcb
    --disable-libxcb_shape
    --disable-libxcb_shm
    --disable-libxcb_xfixes
    --disable-sdl2
    --disable-hwaccels
    --disable-protocols
    --enable-protocol=file,http,tcp,httpproxy
    --disable-muxers
    --disable-demuxers
    --enable-demuxer=matroska,mov,avi,flv,mpegts,mpegtsraw,mpegps,ivf
    --enable-demuxer=m4v,h263,h264,hevc,ingenient,mjpeg,mpegvideo,rawvideo
    --enable-demuxer=mp3,wav,aac,ape,flac,amr,ogg,dsf,iff
    --enable-demuxer=pcm_alaw,pcm_f32be,pcm_f32le,pcm_f64be,pcm_f64le,pcm_mulaw,pcm_s16be,pcm_s16le,pcm_s24be,pcm_s24le,pcm_s32be,pcm_s32le,pcm_s8,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8,sln,adx,g722,gsm,loas
    --disable-parsers
    --enable-parser=h263,h264,hevc,mjpeg,mpeg4video,mpegvideo,vp3,vp8,vp9
    --enable-parser=adx,aac,aac_latm,flac,g729,gsm,mpegaudio,opus,vorbis
    --disable-encoders
    --disable-decoders
    --enable-decoder=h263,h263p,h264,libopenh264,h264_mediacodec,h264_mmal,h264_vda,h264_vdpau,hevc,hevc_mediacodec,mjpeg,rawvideo
    --enable-decoder=mpeg1_vdpau,mpeg1video,mpeg2_mmal,mpeg2video,mpeg4,mpeg4_mmal,mpeg4_vdpau,mpeg4_mediacodec,mpeg_vdpau,mpeg_xvmc,mpegvideo
    --enable-decoder=vp8,vp9,libvpx_vp8,libvpx_vp9,vp8_mediacodec,vp9_mediacodec
    --enable-decoder=pcm_alaw,pcm_alaw_at,pcm_bluray,pcm_dvd,pcm_f32be,pcm_f32le,pcm_f64be,pcm_f64le,pcm_lxf,pcm_mulaw,pcm_mulaw_at,pcm_s16be,pcm_s16be_planar,pcm_s16le,pcm_s16le_planar,adpcm_ima_dat4,adpcm_mtaf
    --enable-decoder=pcm_s24be,pcm_s24daud,pcm_s24le,pcm_s24le_planar,pcm_s32be,pcm_s32le,pcm_s32le_planar,pcm_s8,pcm_s8_planar,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_u8
    --enable-decoder=mp1,mp1_at,mp1float,mp2,mp2float,mp2_at,mp3,mp3adu,mp3adufloat,mp3float,mp3on4,mp3on4float,mp3_at
    --enable-decoder=aac,aac_at,aac_fixed,libfdk_aac,aac_latm,alac,alac_at,als,amrnb,libopencore_amrnb,amr_nb_at,amrwb,libopencore_amrwb,ape,flac,vorbis,libvorbis,libopus,opus
    --enable-decoder=adpcm_4xm,adpcm_adx,adpcm_afc,adpcm_aica,adpcm_ct,adpcm_dtk,adpcm_g722,adpcm_g726,adpcm_g726le
    --enable-decoder=adpcm_ima_amv,adpcm_ima_apc,adpcm_ima_iss,adpcm_ima_oki,adpcm_ima_rad
    --enable-decoder=adpcm_ima_wav,adpcm_psx,adpcm_sbpro_2,adpcm_sbpro_3,adpcm_sbpro_4,adpcm_thp,adpcm_thp_le,adpcm_xa,adpcm_yamaha
    --enable-decoder=g723_1,g729,gsm,libgsm
    --enable-decoder=dsd_lsbf,dsd_msbf,dsd_msbf_planar,dsd_lsbf_planar
"


FF_CONFIG_OPTIONS=`echo $FF_CONFIG_OPTIONS`

./configure ${FF_CONFIG_OPTIONS}
sed -i 's/HAVE_SYSCTL 1/HAVE_SYSCTL 0/g' config.h
sed -i 's/HAVE_SYSCTL=yes/!HAVE_SYSCTL=yes/g' ./ffbuild/config.mak
sed -i 's/HAVE_GLOB 1/HAVE_GLOB 0/g' config.h
sed -i 's/HAVE_GLOB=yes/!HAVE_GLOB=yes/g' config.h

tmp_file=".tmpfile"
## remove invalid restrict define
sed 's/#define av_restrict restrict/#define av_restrict/' ./config.h >$tmp_file
mv $tmp_file ./config.h

## replace original FFMPEG_CONFIGURATION define with $FF_CONFIG_OPTIONS
sed '/^#define FFMPEG_CONFIGURATION/d' ./config.h >$tmp_file
mv $tmp_file ./config.h
total_line=`wc -l ./config.h | cut -d' ' -f 1`
tail_line=`expr $total_line - 3`
head -3 config.h > $tmp_file
echo "#define FFMPEG_CONFIGURATION \"${FF_CONFIG_OPTIONS}\"" >> $tmp_file
tail -$tail_line config.h >> $tmp_file
mv $tmp_file ./config.h

rm -f config.err

## rm BUILD_ROOT information
sed '/^BUILD_ROOT=/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak

## rm amr-eabi-gcc
sed '/^CC=arm-eabi-gcc/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak

## rm amr-eabi-gcc
sed '/^AS=arm-eabi-gcc/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak


## rm amr-eabi-gcc
sed '/^LD=arm-eabi-gcc/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak

## rm amr-eabi-gcc
sed '/^DEPCC=arm-eabi-gcc/d' ./ffbuild/config.mak > $tmp_file
rm -f ./ffbuild/config.mak
mv $tmp_file ./ffbuild/config.mak

sed -i 's/restrict restrict/restrict /g' config.h

sed -i '/getenv(x)/d' config.h
sed -i 's/HAVE_DOS_PATHS 0/HAVE_DOS_PATHS 1/g' config.h

## other work need to be done manually
cat <<!EOF
#####################################################
                    ****NOTICE**** 
You need to modify the file config.mak and delete 
all full path string in macro:
SRC_PATH, SRC_PATH_BARE, BUILD_ROOT, LDFLAGS.
Please refer to the old version of config.mak to 
check how to modify it.
#####################################################
!EOF
