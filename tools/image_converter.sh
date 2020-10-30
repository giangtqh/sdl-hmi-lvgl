#!/bin/bash
cd $(dirname $BASH_SOURCE)
############################################################
# Image converter offline
############################################################
# Required Tool: php7.2-cli
# Optional arguments:
#   - cf color format. Possible values are: true_color, true_color_alpha, true_color_chroma, indexed_1, indexed_2, indexed_4, indexed_8, alpha_1, alpha_2, alpha_4, alpha_8, raw, raw_alpha, raw_chroma. The default is: true_color.
#   - format C array or Binary output. Possible values are: c_array, bin_332, bin_565, bin_565_swap, bin_888. Default is: c_array.
ARG_COLORFORMAT="alpha_8"
ARG_FORMAT="c_array"
TOOL="php"
TOOL_PROG="lv_utils/img_conv_core.php"
ASSETS_PATH_ORIG="../gui/assets/original"
ASSETS_PATH_CONV="../gui/assets/convert"
LIST_IMGS=`ls $ASSETS_PATH_ORIG`
############################################################

FN_Banner() {
	arg1=$1
	echo -e "\e[42m$arg1$\033[0m"
}

FN_Check() {
    if ! command -v $TOOL &> /dev/null; then
        FN_Banner "$TOOL is not supported. Now instaling..."
        sudo apt-get install php7.2-cli
    else
        FN_Banner "$TOOL supported."
    fi
}

FN_Convert() {
    for FILE in $LIST_IMGS ; do
        FN_Banner "Converted $FILE with $ARG_COLORFORMAT format to $ARG_FORMAT"
        $TOOL $TOOL_PROG "name=${FILE%.*}&img=$ASSETS_PATH_ORIG/$FILE&format=$ARG_FORMAT&cf=$ARG_COLORFORMAT"
    done
}

FN_Transfer() {
    mv *.c $ASSETS_PATH_CONV
}

FN_Check
FN_Convert
FN_Transfer
############################################################
