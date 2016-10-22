# Configure paths for GAUTOMATA
# Stolen from glib 

dnl AM_PATH_GAUTOMATA([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GAUTOMATA, and define GAUTOMATA_CFLAGS and GAUTOMATA_LIBS
dnl
AC_DEFUN(AM_PATH_GAUTOMATA,
[dnl 
dnl Get the cflags and libraries from the gautomata-config script
dnl
AC_ARG_WITH(gautomata-prefix,[  --with-gautomata-prefix=PFX   Prefix where GAUTOMATA is installed (optional)],
            gautomata_config_prefix="$withval", gautomata_config_prefix="")
AC_ARG_WITH(gautomata-exec-prefix,[  --with-gautomata-exec-prefix=PFX Exec prefix where GAUTOMATA is installed (optional)],
            gautomata_config_exec_prefix="$withval", gautomata_config_exec_prefix="")

  if test x$gautomata_config_exec_prefix != x ; then
     gautomata_config_args="$gautomata_config_args --exec-prefix=$gautomata_config_exec_prefix"
     if test x${GAUTOMATA_CONFIG+set} != xset ; then
        GAUTOMATA_CONFIG=$gautomata_config_exec_prefix/bin/gautomata-config
     fi
  fi
  if test x$gautomata_config_prefix != x ; then
     gautomata_config_args="$gautomata_config_args --prefix=$gautomata_config_prefix"
     if test x${GAUTOMATA_CONFIG+set} != xset ; then
        GAUTOMATA_CONFIG=$gautomata_config_prefix/bin/gautomata-config
     fi
  fi

  AC_PATH_PROG(GAUTOMATA_CONFIG, gautomata-config, no)
  min_gautomata_version=ifelse([$1], ,0.1,$1)
  AC_MSG_CHECKING(for GAUTOMATA - version >= $min_gautomata_version)
  no_gautomata=""
  if test "$GAUTOMATA_CONFIG" = "no" ; then
    no_gautomata=yes
  else
    GAUTOMATA_CFLAGS=`$GAUTOMATA_CONFIG $gautomata_config_args --cflags`
    GAUTOMATA_LIBS=`$GAUTOMATA_CONFIG $gautomata_config_args --libs`
    gautomata_config_major_version=`$GAUTOMATA_CONFIG $gautomata_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gautomata_config_minor_version=`$GAUTOMATA_CONFIG $gautomata_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gautomata_config_micro_version=`$GAUTOMATA_CONFIG $gautomata_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  fi
  if test "x$no_gautomata" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)

     GAUTOMATA_CFLAGS=""
     GAUTOMATA_LIBS=""
     ifelse([$3], , :, [$3])
  fi

  AC_SUBST(GAUTOMATA_CFLAGS)
  AC_SUBST(GAUTOMATA_LIBS)
  rm -f conf.gautomatatest
])
