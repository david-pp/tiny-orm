#!/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import getopt
import codecs
import commands
import xml.etree.ElementTree as ET

#############################################
#
# 支持的基本类型(C++类型、PROTO类型、DB字段类型、默认值)
#
#############################################
scalar_types = {

    # 整数-有符号
    "int8" : ("int8_t",  "sint32", "INT8", "0"),
    "int16": ("int16_t", "sint32", "INT16", "0"),
    "int32": ("int32_t", "sint32", "INT32", "0"),
    "int64": ("int64_t", "sint64", "INT64", "0"),

    # 整数-无符号
    "uint8" : ("uint8_t",  "uint32", "UINT8", "0"),
    "uint16": ("uint16_t", "uint32", "UINT16", "0"),
    "uint32": ("uint32_t", "uint32", "UINT32", "0"),
    "uint64": ("uint64_t", "uint64", "UINT64", "0"),

    # 浮点
    "float" : ("float", "float", "FLOAT", "0"),
    "double": ("double", "double", "DOUBLE", "0"),

    # 布尔
    "bool": ("bool", "bool", "BOOL", "0"),

    # 字符串
    "string": ("std::string", "bytes", "STRING", ""),
    "vchar" : ("std::string", "bytes", "VCHAR", ""),

    # 二进制
    "bytes"         : ("std::string", "bytes", "BYTES", ""),
    "bytes_tiny"    : ("std::string", "bytes", "BYTES_TINY", ""),
    "bytes_medium"  : ("std::string", "bytes", "BYTES_MEDIUM", ""),
    "bytes_long"    : ("std::string", "bytes", "BYTES_LONG",""),

    # 用户自定义
    # "user-defined"  : ("user-defined", "bytes", "OBJECT")
}

#############################################
#
# 结构体字段
#
#############################################
class FieldDescriptor:
    def __init__(self):
        # 字段编号
        self.num = 0
        # 字段名
        self.name = ''
        # 对应的数据库字段名
        self.dbname = ''
        # 字段类型
        self.type = ''
        # 对应的C++类型
        self.type_cpp = ''
        # 对应的protobuf类型
        self.type_proto = ''
        # 对应的SQL类型
        self.type_sql = ''
        # 默认值
        self.default_specified = False
        self.default = ''
        # 注释
        self.comment = ''

        # 字段大小(仅当type="vchar"时有效)
        self.size = 0

    def __str__(self):
        return "%s,%s,%s" % (self.name, self.type, self.comment.encode('utf-8'))

    def setTypes(self):
        if len(self.type) > 0:
            # 基本类型
            if scalar_types.has_key(self.type):
                self.type_cpp = scalar_types[self.type][0]
                self.type_proto = scalar_types[self.type][1]
                self.type_sql = scalar_types[self.type][2]
                if not self.default_specified :
                    self.default = scalar_types[self.type][3]
            # 用户自定义类型
            else:
                self.type_cpp = self.type.replace('{', '<').replace('}', '>')
                self.type_proto = 'bytes'
                self.type_sql = 'OBJECT'


    def parseFromXML(self, node):
        self.name = node.tag
        self.dbname = self.name.upper()

        if not node.attrib.has_key('type'):
            print >> sys.stderr, '[ERROR] ', self.name, ' type is not defined'
            return False

        if not node.attrib.has_key('num'):
            print >> sys.stderr, '[ERROR] ', self.name, ' num is not defined'
            return False

        if node.attrib.has_key('default'):
            self.default_specified = True
            self.default = node.attrib['default']
        else:
            self.default_specified = False

        self.num = int(node.attrib['num'])
        self.type = node.attrib['type'].strip()
        self.setTypes()

        if self.type == 'vchar':
            if node.attrib.has_key('size'):
                self.size = int(node.attrib['size'])

            if self.size == 0:
                print >> sys.stderr, '[ERROR] ', self.name, ' vchar must specify size="xxx"'
                return False

        if node.attrib.has_key('comment'):
            self.comment = node.attrib['comment']

        return True


#############################################
#
# 结构体描述信息
#
#############################################
class StructDescription:
    def __init__(self):
        # 结构名称
        self.name = ''
        # 对应的数据库表名
        self.dbname = ''
        # 结构字段
        self.fields = []
        self.fields_by_num = {}
        # 主键
        self.primary_keys = []
        # 索引
        self.indexs = []
        # 注释
        self.comment = ''
        # 需要生成ORM
        self.need_orm = True

    def parseFromXML(self, node):
        self.name = node.tag
        self.dbname = self.name.upper()

        if node.attrib.has_key('comment'):
            self.comment = node.attrib['comment']

        if node.attrib.has_key('orm'):
            if node.attrib['orm'].strip() == '0':
                self.need_orm = False

        for subnode in node:
            fd = FieldDescriptor()
            if fd.parseFromXML(subnode):
                if not self.fields_by_num.has_key(fd.num) :
                    self.fields.append(fd)
                    self.fields_by_num[fd.num] = fd
                else:
                    print >> sys.stderr, '[ERROR] ', self.name, fd.name, 'num is already exists!'
                    return False
            else:
                print >> sys.stderr, '[ERROR] ', self.name, subnode.tag, 'parse failed!'
                return False

        if node.attrib.has_key('keys'):
            for key in node.attrib['keys'].split(','):
                if len(key.strip()) > 0 :
                    self.primary_keys.append(key.strip().upper())

        if node.attrib.has_key('index'):
            for key in node.attrib['index'].split(','):
                if len(key.strip()) > 0 :
                    self.indexs.append(key.strip().upper())

        if len(self.primary_keys) == 0:
            print >> sys.stderr, '[ERROR] ', self.name, 'must has a key at least!'
            return False

        return True

    def generateStruct(self, depth, file=sys.stdout):
        printline(file, depth, '')
        if len(self.comment) > 0:
            printline(file, depth, '//')
            printline(file, depth, '// %s' % self.comment)
            printline(file, depth, '//')
        else:
            printline(file, depth, '')

        printline(file, depth, 'struct %s {' % self.name)
        printline(file, depth + 1, '//')
        printline(file, depth + 1, '// Serialization Support')
        printline(file, depth + 1, '//')
        printline(file, depth + 1, 'std::string serialize() const;')
        printline(file, depth + 1, 'bool deserialize(const std::string &data);')
        printline(file, depth + 1, '')

        for f in self.fields:
            if len(f.comment):
                printline(file, depth + 1, '// %s' % f.comment)

            if len(f.default):
                if f.type == 'string' or f.type == 'vchar':
                    printline(file, depth + 1, '%s %s = "%s";' % (f.type_cpp, f.name, f.default))
                else:
                    printline(file, depth + 1, '%s %s = %s;' % (f.type_cpp, f.name, f.default))
            else:
                printline(file, depth + 1, '%s %s;' % (f.type_cpp, f.name))

        printline(file, depth, '};')

    def generateProto(self, depth, file=sys.stdout):
        printline(file, depth, '')
        printline(file, depth, 'message %sProto {' % self.name)
        for f in self.fields:
            printline(file, depth + 1, 'optional %s %s = %d;' % (f.type_proto, f.name, f.num))
        printline(file, depth, '};')
        printline(file, depth, '')

    def generateORM(self, depth, file=sys.stdout):
        printline(file, depth, '')
        printline(file, depth, 'RUN_ONCE(%sORM) {' % self.name)
        printline(file, depth + 1, 'using namespace tiny;')
        printline(file, depth + 1, 'TableFactory::instance().table<%s>("%s")' % (self.name, self.dbname))
        for f in self.fields:
            if f.size > 0:
                printline(file, depth + 2, '.field(&%s::%s, "%s", FieldType::%s, "%s", %d)' % (
                    self.name, f.name, f.dbname, f.type_sql, f.default, f.size))
            else :
                if len(f.default) > 0:
                    printline(file, depth + 2, '.field(&%s::%s, "%s", FieldType::%s, "%s")' % (
                        self.name, f.name, f.dbname, f.type_sql, f.default))
                else:
                    printline(file, depth + 2, '.field(&%s::%s, "%s", FieldType::%s)' % (
                        self.name, f.name, f.dbname, f.type_sql))


        for index in self.indexs:
            printline(file, depth + 2, '.index("%s")' % index)

        for i in range(len(self.primary_keys)):
            if i == (len(self.primary_keys) - 1):
                printline(file, depth + 2, '.key("%s");' % self.primary_keys[i])
            else:
                printline(file, depth + 2, '.key("%s")' % self.primary_keys[i])

        printline(file, depth, '}')

    def generateSerialize(self, depth, file=sys.stdout):
        printline(file, depth, '')
        printline(file, depth, 'std::string %s::serialize() const {' % self.name)
        printline(file, depth + 1, '%sProto proto;' % self.name)
        for f in self.fields:
            if f.type_sql == "OBJECT":
                printline(file, depth + 1, 'proto.set_%s(::serialize(%s));' % (f.name, f.name))
            else:
                printline(file, depth + 1, 'proto.set_%s(%s);' % (f.name, f.name))

        printline(file, depth + 1, 'return proto.SerializeAsString();')
        printline(file, depth, '}')
        printline(file, depth, '')

    def generateDeserialize(self, depth, file=sys.stdout):
        printline(file, depth, '')
        printline(file, depth, 'bool %s::deserialize(const std::string &data) {' % self.name)
        printline(file, depth + 1, '%sProto proto;' % self.name)
        printline(file, depth + 1, 'if (!proto.ParseFromString(data)) return false;')
        for f in self.fields:
            if f.type_sql == "OBJECT":
                printline(file, depth + 1, '::deserialize(%s, proto.%s());' % (f.name, f.name))
            else:
                printline(file, depth + 1, '%s = proto.%s();' % (f.name, f.name))

        printline(file, depth + 1, 'return true;')
        printline(file, depth, '}')
        printline(file, depth, '')


#############################################
#
# 配置文件描述
#
#############################################
class FileDescription:
    def __init__(self):
        # 文件路径
        self.filepath = ''
        # 文件名
        self.filename = ''
        # 依赖的文件
        self.imports = []
        # 包含的的结构体描述符
        self.descriptors = []

    def parseFromXML(self, xmlfile):
        self.filepath = xmlfile
        self.filename = os.path.splitext(os.path.basename(self.filepath))[0]
        try:
            tree = ET.ElementTree(file=self.filepath)
            root = tree.getroot()

            for object in root:
                if object.tag == 'import':
                    self.imports.append(os.path.splitext(os.path.basename(object.text))[0])
                else :
                    descriptor = StructDescription()
                    if descriptor.parseFromXML(object):
                        self.descriptors.append(descriptor)
                    else:
                        print >> sys.stderr, '[ERROR] ', self.filepath, 'parse failed !'
                        return False
            # print self.imports
            return True
        except IOError, arg:
            print arg

    def generateHeader(self):
        filepath = output_dir + '/' + self.filename + '.h'
        outfile = codecs.open(filepath, 'w+', "utf-8")
        writeLicence(outfile)
        printline(outfile, 0, "#ifndef __TINYOBJ_%s__" % self.filename.upper())
        printline(outfile, 0, "#define __TINYOBJ_%s__" % self.filename.upper())
        printline(outfile, 0, "")
        printline(outfile, 0, '#include "tinyobj.h"')
        for file in self.imports:
            printline(outfile, 0, '#include "%s.h"' % file)
        printline(outfile, 0, "")
        printline(outfile, 0, "namespace tiny {")
        for desc in self.descriptors:
            desc.generateStruct(0, outfile)

        printline(outfile, 0, "} // namespace tiny")
        printline(outfile, 0, "#endif // __TINYOBJ_%s__" % self.filename.upper())

        print >> sys.stderr, '[INFO] ', filepath, 'is generated.'


    def generateSource(self):
        filepath = output_dir + '/' + self.filename + '.cpp'
        outfile = codecs.open(filepath, 'w+', "utf-8")
        printline(outfile, 0, '#include "%s.h"' % self.filename)
        printline(outfile, 0, '#include "%s.pb.h"' % self.filename)
        printline(outfile, 0, "")
        printline(outfile, 0, "namespace tiny {")
        for desc in self.descriptors:
            desc.generateSerialize(0, outfile)
            desc.generateDeserialize(0, outfile)
        printline(outfile, 0, "} // namespace tiny")

        for desc in self.descriptors:
            if desc.need_orm:
                desc.generateORM(0, outfile)

        print >> sys.stderr, '[INFO] ', filepath, 'is generated.'

    def generateProto(self):
        filepath = output_dir + '/' + self.filename + '.proto'
        outfile = codecs.open(filepath, 'w+', "utf-8")
        printline(outfile, 0, 'syntax = "proto2";')
        for file in self.imports:
            printline(outfile, 0, 'import "%s.proto";' % file)
        printline(outfile, 0, "")
        printline(outfile, 0, 'package tiny;')
        printline(outfile, 0, "")
        for desc in self.descriptors:
            desc.generateProto(0, outfile)

        print >> sys.stderr, '[INFO] ', filepath, 'is generated.'

        (status, output) = commands.getstatusoutput('protoc -I=%s --cpp_out=%s %s' % (output_dir, output_dir, filepath))
        if status == 0:
            print >> sys.stderr, '[INFO] ', filepath, '-> %s.pb.h & %s.pb.cc' % (self.filename, self.filename)
        else :
            print >> sys.stderr, status, output
            sys.exit(1)


def test_primitive():
    print primitive.getTypeDef("int32")
    print primitive.getTypeDef("char[32]")


def printline(file, depth, text):
    """print line with indent"""
    for i in range(0, depth):
        print >> file, '    ',
    print >> file, text

def writeLicence(file=sys.stdout):
    file.write("""
// MIT License
// 
// Copyright (c) 2017 david++
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//
// !!NOTICE: This file is generated by TOOL, DON'T EDIT!. (by david++)")
//
""")

def parse(filepath):
    fd = FileDescription()
    if fd.parseFromXML(filepath):
        fd.generateHeader()
        fd.generateProto()
        fd.generateSource()

#######################################################
#
# Main
#
#######################################################

output_dir = os.getcwd()

def Usage():
    print "usage:", sys.argv[0], "-o outputdir obj1.xml obj2.xml ..."
    print "options: "
    print "  -h --help             help"
    print "  -o dir --output=dir   output to the dir"

if len(sys.argv) == 1:
    Usage()
    sys.exit()

try:
    opts, args = getopt.getopt(sys.argv[1:], 'ho:', ['help', 'output='])
except getopt.GetoptError:
    Usage()
    sys.exit()

for o, a in opts:
    if o in ("-h", "--help"):
        Usage()
        sys.exit()
    elif o in ("-o", "--output"):
        output_dir = a

if len(args) > 0:
    for file in args:
        parse(file);
else:
    Usage()
