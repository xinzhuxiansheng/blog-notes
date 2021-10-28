1. Standalone模式启动 start-cluster.sh报错
```
# sh start-cluster.sh
/data/installDirs/flink-1.14.0/bin/config.sh: line 32: syntax error near unexpected token `<'
/data/installDirs/flink-1.14.0/bin/config.sh: line 32: `    done < <(find "$FLINK_LIB_DIR" ! -type d -name '*.jar' -print0 | sort -z)'
```

不要使用sh start-cluster.sh运行，直接运行 ./start-cluster.sh