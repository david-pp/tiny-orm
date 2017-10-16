TinyORM
--------------------

对象关系映射（ORM，Object-Relational Mappings），是一种程序技术，用于实现面向对象编程语言里不同类型系统的数据之间的转换。说白了，就是将C++对象映射到数据库表的一种技术，目的就是简化对象存档和交换的代码。

一般情况下，对象要存储到关系型数据库，需要在代码里面使用SQL语句，若对象较为复杂的话，就需要编写大量代码去支持对象的数据库操作，代码也不怎么美观，扩展性也比较差。ORM则封装这些操作作为对象的成员函数，这样使用时候就会比较方便。（PS. ORM会损失一定的灵活性，偶尔SQL也是必要的）

 ![TinyORM](./docs/tinyorm.png)

## 1. 用法(How to use ?)


## 2. 设计(How to work ?)
 
## 0. 依赖

 - `C++11`
 - `protobuf`
 - `mysql/mysql++` 或 `soci`
 
 ## 参考
 
 https://en.wikipedia.org/wiki/Object-relational_mapping
 
 