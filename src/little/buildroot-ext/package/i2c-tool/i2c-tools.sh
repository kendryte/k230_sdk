#/bin/sh

slave_dev=0x70
fail_count=0

test_i2c_bus()
{
  echo "cmd:i2cdetect -l |grep "i2c-0""
  i2cdetect -l |grep "i2c-0"
  if [ $? -eq 0 ];then
     echo "i2c bus contain i2c-0 test_result:Pass"
  else
     echo "i2c bus contain i2c-0 test_result:Fail"
     let fail_count++
  fi    
}

test_i2c_dev_mount()
{
  echo "cmd:i2cdetect -r -y 0 |grep "70 ""
  i2cdetect -r -y 0 |grep "70 "
  if [ $? -eq 0 ];then
     echo "i2c-0 already mount 0x70 slave dev test_result:Pass"
  else
     echo "i2c-0 already mount 0x70 slave dev test_result:Fail"
     let fail_count++
  fi    
}


test_i2c_dev_dump()
{
  echo "cmd:i2cdump -f -y 0 0x70"
  i2cdump -f -y 0 0x70
  if [ $? -eq 0 ];then
     echo "i2cdump test_result:Pass"
  else
     echo "i2cdump test_result:Fail"
     let fail_count++
  fi    
}
i2c_write_read()
{
   addr=$1
   set_val=$2   
   echo "i2c write value:$set_val"
   i2cset -f -y 0 $slave_dev $addr $set_val || echo "fail:i2cset command exec fail"
   get_val=`i2cget -f -y 0 0x70 $addr` ||echo "fail:i2cget command exec fail"
   echo "i2c read value:$get_val"
   if [ "$set_val" != "$get_val" ]; then
       echo "i2c_write_read test_reult:Fail"
       let fail_count++
       return 1
    fi
}

test_i2c_write_read()
{
  echo "i2c write and read test"
  i2c_write_read 0x1 0x02 && i2c_write_read 0x1 0x03 && i2c_write_read 0x2 0x0b && i2c_write_read 0x2 0x0c && i2c_write_read 0x3 0x1b && i2c_write_read 0x3 0x1c &&  echo "i2c_write_read test_reult:Pass"
}

main()
{
  test_i2c_bus
  test_i2c_dev_mount
  test_i2c_dev_dump
  test_i2c_write_read
  echo "Total test fail count:$fail_count"
}
main

