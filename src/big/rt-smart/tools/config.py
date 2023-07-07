#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

def ParseConfig(configfile):
    conf = {}

    try:
        f = open(configfile)
        for line in f:
            line = line.strip()
            if line.startswith('#'):
                continue
            elif line.find('=') == -1:
                continue
            else:
                item = line.split('=')
                if item[0].startswith('CONFIG_'):
                    item[0] = item[0].replace('CONFIG_', '')

                if item[1] == 'y':
                    conf[item[0]] = True
                elif item[1].startswith('0x'):
                    conf[item[0]] = int(item[1], 16)
                elif item[1].startswith('"'):
                    conf[item[0]] = item[1].replace('"', '')
                else:
                    conf[item[0]] = int(item[1])
    except:
        print('Parse Configuration file failed')

    return conf

if __name__ == '__main__':
    conf = ParseConfig('.config')
    print(conf)
