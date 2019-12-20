#!/bin/bash
#####################################################################
# brief: 将产出打包到output.tar.gz && 并在output目录下生成运行目录
#####################################################################
OUTPUTDIR="output"
DIRS=("bin" "bin/python" "conf" "imgs")
EXECFILES=("vpsnr" "checkdropframe")
READINGFILES=("README.md" "LICENSE")

rm -rf ${OUTPUTDIR}

for((i=0; i<${#DIRS[@]}; i++))
do
	mkdir -p ${OUTPUTDIR}/${DIRS[i]}/
done

for((i=0; i<${#EXECFILES[@]}; i++))
do
	cp -rf ${EXECFILES[i]} ${OUTPUTDIR}/bin/
done

for((i=0; i<${#READINGFILES[@]}; i++))
do
	cp -rf ${READINGFILES[i]} ${OUTPUTDIR}/
done

# Deploy shell python conf
cp -rf *.sh ${OUTPUTDIR}/bin

cp -rf python ${OUTPUTDIR}/bin/

cp -rf conf/* ${OUTPUTDIR}/conf/


tar -cf output.tar.gz output/

