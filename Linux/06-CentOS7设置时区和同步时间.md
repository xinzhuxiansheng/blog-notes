## 设置时区
第一步：查询服务器时间
```shell
[root@localhost ~]# timedatectl
        Local time: Sat 2018-03-31 01:11:46 UTC
    Universal time: Sat 2018-03-31 01:11:46 UTC
        RTC time: Sat 2018-03-31 01:11:36
        Time zone: Universal (UTC, +0000)
        NTP enabled: yes
NTP synchronized: yes
    RTC in local TZ: no
        DST active: n/a
```

第二步；修改时区为Asia/Shanghai
```shell
[root@localhost ~]# timedatectl  set-timezone Asia/Shanghai
```

第三步：查看修改后的结果
```shell
[root@localhost ~]# timedatectl
        Local time: Sat 2018-03-31 09:13:21 CST
    Universal time: Sat 2018-03-31 01:13:21 UTC
        RTC time: Sat 2018-03-31 01:13:11
        Time zone: Asia/Shanghai (CST, +0800)
        NTP enabled: yes
NTP synchronized: yes
    RTC in local TZ: no
        DST active: n/a
```


## 同步时间

```shell
* * * * * /usr/sbin/ntpdate time.pool.aliyun.com >> /usr/local/time.log
```


## 设置时间
从 Centos7开始，使用新命令 `timedatectl`

### timedatectl命令介绍
1. 读取时间

timedatectl //等同于 timedatectl status

2. 设置时间

timedatectl set-time "YYYY-MM-DD HH:MM:SS"

3. 列出所有时区

timedatectl list-timezones

4. 设置时区

timedatectl set-timezone Asia/Shanghai

5. 是否NTP服务器同步

timedatectl set-ntp yes //yes或者no

6. 将硬件时钟调整为与本地时钟一致

timedatectl set-local-rtc 1
hwclock --systohc --localtime //与上面命令效果一致

注意: 硬件时钟默认使用UTC时间，因为硬件时钟不能保存时区和夏令时调整，修改后就无法从硬件时钟中读取出准确标准时间，因此不建议修改。修改后系统会出现下面的警告。

GMT、UTC、CST、DST 时间

(1) UTC

整个地球分为二十四时区，每个时区都有自己的本地时间。在国际无线电通信场合，为了统一起见，使用一个统一的时间，称为通用协调时(UTC, Universal Time Coordinated)。

(2) GMT

格林威治标准时间 (Greenwich Mean Time)指位于英国伦敦郊区的皇家格林尼治天文台的标准时间，因为本初子午线被定义在通过那里的经线。(UTC与GMT时间基本相同，本文中不做区分)

(3) CST

中国标准时间 (China Standard Time)

(4) DST

夏令时(Daylight Saving Time) 指在夏天太阳升起的比较早时，将时钟拨快一小时，以提早日光的使用。（中国不使用）

GMT + 8 = UTC + 8 = CST
