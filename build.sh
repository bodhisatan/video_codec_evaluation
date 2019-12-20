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
echo "Create dir success"

for((i=0; i<${#EXECFILES[@]}; i++))
do
	cp -rf ${EXECFILES[i]} ${OUTPUTDIR}/bin/
done
echo "Deploy exec file success"

for((i=0; i<${#READINGFILES[@]}; i++))
do
	cp -rf ${READINGFILES[i]} ${OUTPUTDIR}/
done
echo "Deploy reading file success"

# Deploy shell python conf
cp -rf *.sh ${OUTPUTDIR}/bin
echo "Deploy shell script success"

cp -rf python ${OUTPUTDIR}/bin/
echo "Deploy python script dsuccess"

cp -rf conf/* ${OUTPUTDIR}/conf/
echo "Deploy conf file success"

tar -cf output.tar.gz output/

