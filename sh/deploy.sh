#!/bin/bash
REPO=http://foo
TAG=${REPO}tags/
URL=${TAG}$1
LOCAL=foofoo

replace_json() {
  IFS=""
  FILENAME=$1
  TEMPFILE=.tmpfile
  KEYWORD=$2
  NEWDATA=$3

  rm -rf ${TEMPFILE}
  mv ${FILENAME} ${TEMPFILE}
  while read line; do
    echo ${line} | grep ${KEYWORD} > /dev/null
    if [ $? -eq 0 ]; then
      echo ${line} | sed 's|\(.*\): \(.*\)|\1:'${NEWDATA}'|' >> ${FILENAME}
    else
      echo ${line} >> ${FILENAME}
    fi
  done < ${TEMPFILE}
}

replace_export() {
  IFS=""
  FILENAME=$1
  TEMPFILE=.tmpfile
  KEYWORD=$2
  NEWDATA=$3

  rm -rf ${TEMPFILE}
  mv ${FILENAME} ${TEMPFILE}
  while read line; do
    echo ${line} | grep ${KEYWORD} > /dev/null
    if [ $? -eq 0 ]; then
      echo ${line} | sed 's|\(.*export\)\(.*\)=\(.*\)|\1\2='${NEWDATA}'|' >> ${FILENAME}
    else
      echo ${line} >> ${FILENAME}
    fi
  done < ${TEMPFILE}
}

comment_out() {
  IFS=""
  FILENAME=$1
  TEMPFILE=.tmpfile
  KEYWORD=$2

  rm -rf ${TEMPFILE}
  mv ${FILENAME} ${TEMPFILE}
  while read line; do
    echo ${line} | grep ${KEYWORD} > /dev/null
    if [ $? -eq 0 ]; then
      echo ";" ${line} >> ${FILENAME}
    else
      echo ${line} >> ${FILENAME}
    fi
  done < ${TEMPFILE}
}

mod_file() {
  FNAME=${LOCAL}/foo
  if [ ! -e ${FNAME} ]; then
    exit ${ECODE_FILE}
  fi
  replace_json ${FNAME} AAA BBB
  replace_export ${FNAME} CCC DDD
  comment_out ${FNAME} EEE
}

################################################################
# main 
################################################################
# 引数がないときは svn ls して終了
if [ $# -eq 0 ]; then
  echo "タグ名を指定してください。今あるタグは..."
  svn ls ${TAG}
  exit 1
fi

# svn 接続存在チェック
svn ls ${URL}
if [ $? -ne 0 ]; then
  echo "接続エラー ${URL}"
  exit 1
fi

# フォルダ存在チェック
if [ -e ${LOCAL} ]; then
  echo "${LOCAL} フォルダが存在しています"
  exit 1
fi

# export する
echo "export しています..."
svn export -q ${URL} ${LOCAL}
if [ $? -ne 0 ]; then
  echo "接続エラー ${URL}"
  exit 1
fi

# ファイルの更新
echo "ファイル変更"
mod_file

