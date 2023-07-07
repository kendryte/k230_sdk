#!/bin/bash

GOLDEN_PATH=/home/QA/kai/liuyuan_20191127_tst_zk/build.s2c/sdk/samples
RESULT_PATH=`pwd`

usage()
{
echo "the init golden path is : ${GOLDEN_PATH}"
echo "please confirm your GOLDEN_PATH at fist.If the GOLDEN_PATH not exist !!!! "
}

run_vglite_case()
{
Vglite_case_list1=$(ls -al | grep "rwx" | grep -v "drwx" | grep -v  ".so" | grep -v  ".sh" |grep -v "unit_test1"|awk -F " " '{print $9}')
for vglite_sub_case1 in ${Vglite_case_list1} 
do 
   case "${vglite_sub_case1}" in 
     ft255)
           mkdir -p  result_bmp
           ./${vglite_sub_case1}
           mv result_bmp result_bmp_ft255
     ;;
     unit_test)
           mkdir -p result_bmp
           ./${vglite_sub_case1}
           mv result_bmp result_bmp_unittest
     ;;
     combine) 
           ./${vglite_sub_case1} gfx1  
     ;;
     *)
           ./${vglite_sub_case1}
     ;;
   esac
done

Vglite_case_list2=$(ls -al |grep "unit_test1"|awk -F " " '{print $9}')
for vglite_sub_case2 in ${Vglite_case_list2}
do 
           mkdir -p result_bmp
           ./${vglite_sub_case2}
           mv result_bmp result_bmp_unittest1
done 

} 

compare_ft255()
{
rm -rf passed_ft255.txt
rm -rf failed_ft255.txt
#the golden path of vglite 
src_path2=${GOLDEN_PATH}/result_bmp_ft255

#the test results of vglite
src_path1=${RESULT_PATH}/result_bmp_ft255

file1=`ls ${src_path1}/SFT_*`
file2=`ls ${src_path2}/SFT_*`

total_result_num=`echo "${file1}" | wc -l`
total_golden_num=`echo "${file2}" | wc -l`
echo "ft255 total result num is :${total_result_num}"
echo "ft255 total golden num is : ${total_golden_num}"


for onecase1 in ${file1}
do
   #echo "onecase1 is ${onecase1}" 
   onecase2=${onecase1/$src_path1/$src_path2}
   #echo "onecase2 is ${onecase2}"
   if [[ ! -f "${onecase1}" || ! -f "${onecase2}" ]]
   then
        echo "the file is not exist"
   else
        result=`diff ${onecase1} ${onecase2}`
        #echo "${result}"
        if [ -z "${result}" ]
        then
              echo " ${onecase1} is passed" >> passed_ft255.txt
        else
            echo "${onecase1} is failed" >> failed_ft255.txt
        fi
   fi
done
passed_num=`cat passed_ft255.txt |grep "passed" | wc -l`
failed_num=${total_result_num}-${passed_num}
pass_rate=`expr "scale=4;$passed_num*100/$total_result_num"| bc`
echo "the ft255 passed count is:${passed_num}, the ft255 failed num is : ${failed_num}, the pass rate is ${pass_rate}%"
}

compare_unittest()
{
rm -rf passed_unittest.txt
rm -rf failed_unittest.txt
#the golden path of vglite 
src_path2=${GOLDEN_PATH}/result_bmp_unittest

#the test results of vglite
src_path1=${RESULT_PATH}/result_bmp_unittest

file1=`ls ${src_path1}/*`
file2=`ls ${src_path2}/*`

total_result_num=`echo "${file1}" | wc -l`
total_golden_num=`echo "${file2}" | wc -l`
echo "unittest total result num is :${total_result_num}"
echo "unittest total golden num is : ${total_golden_num}"


for onecase1 in ${file1}
do
   #echo "onecase1 is ${onecase1}" 
   onecase2=${onecase1/$src_path1/$src_path2}
   #echo "onecase2 is ${onecase2}"
   if [[ ! -f "${onecase1}" || ! -f "${onecase2}" ]]
   then
        echo "the file is not exist"
   else
        result=`diff ${onecase1} ${onecase2}`
        #echo "${result}"
        if [ -z "${result}" ]
        then
              echo " ${onecase1} is passed" >> passed_unittest.txt
        else 
              echo "${onecase1} is failed" >> failed_unittest.txt     
        fi
   fi
done
passed_num=`cat passed_unittest.txt |grep "passed" | wc -l`
failed_num=${total_result_num}-${passed_num}
pass_rate=`expr "scale=4;$passed_num*100/$total_result_num"| bc`
echo "the unittest passed count is:${passed_num}, the unittest failed num is : ${failed_num}, the pass rate is ${pass_rate}%"
}

compare_unittest1()
{
rm -rf passed_unittest1.txt
rm -rf failed_unittest1.txt
#the golden path of vglite 
src_path2=${GOLDEN_PATH}/result_bmp_unittest1

#the test results of vglite
src_path1=${RESULT_PATH}/result_bmp_unittest1

file1=`ls ${src_path1}/*`
file2=`ls ${src_path2}/*`

total_result_num=`echo "${file1}" | wc -l`
total_golden_num=`echo "${file2}" | wc -l`
echo "unittest1 total result num is :${total_result_num}"
echo "unittest1 total golden num is : ${total_golden_num}"

for onecase1 in ${file1}
do
   #echo "onecase1 is ${onecase1}" 
   onecase2=${onecase1/$src_path1/$src_path2}
   #echo "onecase2 is ${onecase2}"
   if [[ ! -f "${onecase1}" || ! -f "${onecase2}" ]]
   then
        echo "the file is not exist"
   else
        result=`diff ${onecase1} ${onecase2}`
        echo "${result}"
        if [ -z "${result}" ]
        then
              echo " ${onecase1} is passed" >> passed_unittest1.txt
        else 
              echo " ${onecase1} is failed" >> failed_unittest1.txt
        fi
   fi
done
passed_num=`cat passed_unittest1.txt |grep "passed" | wc -l`
failed_num=${total_result_num}-${passed_num}
pass_rate=`expr "scale=4;$passed_num*100/$total_result_num"| bc`
echo "the unittest1 passed count is:${passed_num}, the unittest1 failed num is : ${failed_num}, the pass rate is ${pass_rate}%"
}

compare_lite_case()
{
#the golden path of vglite 
src_path2=${GOLDEN_PATH}

#the test results of vglite
src_path1=${RESULT_PATH}

file1=`ls ${src_path1}/*.png`
file2=`ls ${src_path2}/*.png`

total_result_num=`echo "${file1}" | wc -l`
total_golden_num=`echo "${file2}" | wc -l`
echo "lite_case total result num is :${total_result_num}"
echo "lite_case total golden num is : ${total_golden_num}"

for onecase1 in ${file1}
do
   #echo "onecase1 is ${onecase1}" 
   onecase2=${onecase1/$src_path1/$src_path2}
   #echo "onecase2 is ${onecase2}"
   if [[ ! -f "${onecase1}" || ! -f "${onecase2}" ]]
   then
        echo "the file is not exist"
   else
        result=`diff ${onecase1} ${onecase2}`
        echo "${result}"
        if [ -z "${result}" ]
        then
              echo " ${onecase1} is passed" 
        else
              echo " ${onecase1} is failed"
        fi
   fi
done
}

usage
run_vglite_case
compare_lite_case
compare_ft255
compare_unittest
compare_unittest1



