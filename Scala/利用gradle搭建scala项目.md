**正文**


## idea 利用gradle创建scala项目步骤 (略)

## 配置新项目
1. 在项目中增加scala文件夹，与java目录平级，并将scala设置成 Sources
2. 配置build.gradle，配置 apply plugin，scala类库的dependencies，这样就不需要在idea设置scala的类库了

```xml
plugins {
    id 'java'
}

group 'com.xinzhuxiansheng.scalaproject'
version '1.0-SNAPSHOT'

apply plugin: 'java' //指定java插件
apply plugin: 'scala' //指定scala插件

sourceCompatibility = 1.8

repositories {
    mavenCentral()
}

dependencies {
    testCompile group: 'junit', name: 'junit', version: '4.12'
    compile group: 'org.scala-lang', name: 'scala-library', version: '2.13.1' //配置scala类库
}
```
