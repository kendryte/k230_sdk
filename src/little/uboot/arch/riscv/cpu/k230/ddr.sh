#!/bin/bash
cat > ddr.awk  <<'EOF'
#使用说明，需要手动修改下指令和数据 部分代码的 行号；
#FNR >= 477 && FNR < 16860   需要修改  //477  -- 16860 reg_write(   DDR_REG_BASE +0x53fff*4+0x02000000,0x0);
#FNR > 16883 && FNR < 17774    需要修改 //16866  --- 

BEGIN { print "wwwwwwwwwinstr---instruction--" > "files.c";}


#指令
FNR >= isl && FNR <= iel &&  /reg_write/ && (! /^\s*\/\//) { 
	match($0, "[^+]*+ *(0x[0-f]*)[^,]*, *(0x[0-f]*)", ary);
	print "//inst " $0 " " FNR " " ary[1] " " ary[2] >>"files.c";
	instrction[ary[1]]=ary[2];
}


#数据 16883 17774
FNR >= dsl && FNR <=del  && /reg_write/ && (! /^\s*\/\//) { 
	match($0, "[^+]*+ *(0x[0-f]*)[^,]*, *(0x[0-f]*)", ary);
	print "//data " $0 " "   FNR " " ary[1] " " ary[2] >>"files.c";
	data[ary[1]]=ary[2];
}

END {#print "end\n ";
	system("rm -rf it inst inst.c dt dtat dtat.c");
	n=asorti(instrction, sorted)
	#printf("instrucnt n=%d %x\n",n,n)

	for (i=1; i<=n; i++) {
		#printf("%s ",sorted[i])>>"it"
		if( (i>1) && ((strtonum(sorted[i])-strtonum(sorted[i-1])) !=1 )) {
			printf ("instr 	error i=%d %s %s\n",i, sorted[i], sorted[i-1] );
			exit 1;
		}
		printf ("%04x ",strtonum(instrction[sorted[i]]) ) >>"inst";
		printf ("0x%04x,",strtonum(instrction[sorted[i]]) ) >>"inst.c";
	}
	system("xxd -r -p inst imem.bin");

	n=asorti(data, sorted)
	#printf("data n=%d %x\n",n,n)
	for (i=1; i<=n; i++) {
		#printf("%s ",sorted[i])>>"dt"
		if( (i>1) && ((strtonum(sorted[i])-strtonum(sorted[i-1])) !=1 )) {
			printf ("data error i=%d %s %s\n",i, sorted[i], sorted[i-1] );
			exit 1;
		}
		printf ("%04x ",strtonum(data[sorted[i]]) )>>"data";
		printf ("0x%04x,",strtonum(data[sorted[i]]) ) >>"data.c";
	}
	system("xxd -r -p data dmem.bin");

}
EOF




#f=$(basename $1)
f=$1
df=$2 #$(basename $2)

#指令
#reg_write(   DDR_REG_BASE +0x50000*4+0x02000000,0xb0); //isl
#reg_write(   DDR_REG_BASE +0x53fff*4+0x02000000,0x0); //iel
#数据
#reg_write(   DDR_REG_BASE +0x54000*4+0x02000000,0x0);//dsl
#reg_write(   DDR_REG_BASE +0x54359*4+0x02000000,0x0);//del

#获取行号
isl=$(grep 0x50000 $f  -n | cut  -d:  -f1);
iel=$(grep 0x53fff $f  -n | cut  -d:  -f1);
dsl=$(grep 0x54000 $f  -n | cut  -d:  -f1);
del=$(grep 0x54359 $f  -n | cut  -d:  -f1);

#转成imem.bin dmem.bin
set -e ; gawk -v isl=${isl}  -v iel=${iel} -v dsl=${dsl}  -v del=${del} -f ddr.awk  $f;
cp $f $df;

#替换成for循环函数；
sed -i -e "${dsl},${del}d"  $df
sed -i -e "${dsl}a ddr_wite_bin(DDR_REG_BASE+0x54000*4+0x02000000,gDdrD,ARRAY_SIZE(gDdrD));"  $df   ##data
sed -i -e "${isl},${iel}d"  $df
sed -i -e "${isl}a ddr_wite_bin(DDR_REG_BASE+0x50000*4+0x02000000,gDdrI,ARRAY_SIZE(gDdrI));"  $df  ##instruction


cat > temp.c  <<EOF
#include <asm/io.h>
#include <linux/kernel.h>
//ddr_wite_bin(0x50000,gDdrI,ARRAY_SIZE(gDdrI));
//ddr_wite_bin(0x54000,gDdrD,ARRAY_SIZE(gDdrD));
static const short gDdrI[]={
	0x11111111,
};
static const short gDdrD[]={
	0xdddddddd,
};
int  ddr_wite_bin(ulong regs,const short *bin,ulong blen)
{
	int i = 0;
	for(i=0;i<blen;i++)
	{
		writel(*(bin+i), (volatile void __iomem *)(regs+i*4));
		//if((i==0)|| ((i+1)==blen))
		//	printf("i=%05d %x %04x %lx\n",i,regs, *(bin+i), regs+i*4);

	}
		
	return 0;
} 
EOF

#指令数组
tl=$(grep 0x11111111 temp.c  -n | cut  -d:  -f1);
sed -i  -e "$tl d"   temp.c
sed -i  -e "$(($tl-1)) r  inst.c"  temp.c
#数据数组
tl=$(grep 0xdddddddd temp.c  -n | cut  -d:  -f1);
sed -i  -e "$tl d"   temp.c
sed -i  -e "$(($tl-1)) r  data.c"  temp.c
#更新代码及头文件；
sed -i -e "/0x98000000/ r temp.c"  $df 


rm -rf  dmem.c  imem.c  inst.c data.c   temp.c  data  inst  

#imem.bin--指令bin文件；
#dmem.bin--数据bin文件；
#files.c--- 查看提前的关键值；
rm -rf imem.bin  dmem.bin files.c  ddr.awk

