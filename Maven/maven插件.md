**`正文`**
[TOC]

## maven-antrun-plugin
* 简介：


* 使用：
```xml
<plugin>
    <artifactId>maven-antrun-plugin</artifactId>
    <version>1.2</version>
    <executions>
        <execution>
            <id>copy-resources</id>
            <!-- here the phase you need -->

            <!--maven的生命周期-->
            <!--<phase>package</phase>-->
            <phase>install</phase>
            <goals>
                <goal>run</goal>
            </goals>
            <configuration>
                <tasks>
                    <move file="${basedir}/target/${project.name}-${project.version}.jar"
                            tofile="${basedir}/../../../plugin/filter/${project.name}-${project.version}.jar" />
                </tasks>
            </configuration>
        </execution>
    </executions>
</plugin>
```


## maven-shade-plugin

* 使用：
```xml
<plugin>
    <groupId>org.apache.maven.plugins</groupId>
    <artifactId>maven-shade-plugin</artifactId>
    <version>3.1.0</version>
    <executions>
        <execution>
            <phase>package</phase>
            <goals>
                <goal>shade</goal>
            </goals>
            <configuration>
                <transformers>
                    <transformer implementation="org.apache.maven.plugins.shade.resource.ManifestResourceTransformer">
                        <mainClass>xx.xxx.xxxxx.xxxx.AdminApiApplication</mainClass>
                    </transformer>
                    <transformer implementation="org.apache.maven.plugins.shade.resource.AppendingTransformer">
                        <resource>reference.conf</resource>
                    </transformer>
                </transformers>
            </configuration>
        </execution>
    </executions>
</plugin>

```


## scala-maven-plugin

* 使用：
```xml
<plugin>
    <groupId>org.scala-tools</groupId>
    <artifactId>maven-scala-plugin</artifactId>
    <version>2.15.2</version>
    <executions>
        <execution>
            <id>scala-compile</id>
            <goals>
                <goal>compile</goal>
            </goals>
            <configuration>
                <!--includes是一个数组，包含要编译的code-->
                <includes>
                    <include>**/*.scala</include>
                </includes>
            </configuration>
        </execution>
        <execution>
            <id>scala-test-compile</id>
            <goals>
                <goal>testCompile</goal>
            </goals>
        </execution>
    </executions>
</plugin>





<plugin>
    <groupId>com.lightbend.akka.grpc</groupId>
    <artifactId>akka-grpc-maven-plugin</artifactId>
    <version>0.5.0</version>
    <executions>
        <execution>
            <goals>
                <goal>generate</goal>
            </goals>
        </execution>
    </executions>
    <configuration>
        <language>Java</language>
        <generateClient>true</generateClient>
        <generateServer>true</generateServer>
        <generatorSettings>
            <serverPowerApis>true</serverPowerApis>
        </generatorSettings>
    </configuration>
</plugin>
```
