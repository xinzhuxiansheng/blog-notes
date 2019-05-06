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


