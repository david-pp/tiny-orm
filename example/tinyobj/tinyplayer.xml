<?xml version="1.0" encoding="UTF-8"?>
<tinyobj>
    <!--<import>tinyplayer.xml</import>-->
    <!--<import>tinyplayer.xml</import>-->
    <!--<import>tinyplayer.xml</import>-->

    <Friend orm="0" keys="id" index="" comment="好友">
        <id      num="1" type="uint32"  comment="标识符"/>
        <name    num="2" type="vchar" size="32"  comment="名字"/>
    </Friend>

    <Country orm="0" keys="id" index="" comment="国家">
        <id      num="1" type="uint32"  comment="标识符"/>
        <name    num="2" type="vchar" size="32"   comment="名字"/>
    </Country>

    <Player keys="id" index="name" comment="角色信息">
        <id      num="1" type="uint32"  comment="标识符"/>
        <name    num="2" type="string"  default="david" comment="名字"/>
        <country num="3" type="Country" comment="国家信息"/>
        <friends num="4" type="std::vector{Friend}" comment="朋友列表"/>
        <m_int8  num="10" type="int8" default="22"/>
        <m_int16 num="11" type="int16"/>
        <m_int32 num="12" type="int32"/>
        <m_int64 num="13" type="int64"/>
        <m_uint8  num="14" type="uint8"/>
        <m_uint16 num="15" type="uint16"/>
        <m_uint32 num="16" type="uint32"/>
        <m_uint64 num="17" type="uint64"/>
        <m_float  num="18" type="float"/>
        <m_double num="19" type="double"/>
        <m_bool   num="20" type="bool"/>
        <m_string num="21" type="string"/>
        <m_bytes  num="22" type="bytes"/>
        <m_bytes_tiny  num="23" type="bytes_tiny"/>
        <m_bytes_medium  num="24" type="bytes_medium"/>
        <m_bytes_long  num="25" type="bytes_long"/>
    </Player>
</tinyobj>