　　crontab 是linux系统集成的定时任务命令，它对于系统运行（数据的备份，调度系统资源或命令，程序的定时触发等非常有用）。crontab有些类似于Windows系统的Service的任务计划程序。它可以指定某个后台程序的执行的时间或者时间间隔。

　　大家可以安装VMware虚机，测试mysql数据的备份，测试webservice服务的定时启动等等，也许你会用上shell编程；

　　那下面开始跟大家说说crontab的使用：

　　[Minute] [Hour] [Month] [Year] [Week] [Command]

　　

　　这里介绍两种方法去执行调度命令：

　　大家先创建SystemDate文件，后面才能  >>  或 先在终端输入 #date > /home/SystemDate

　　a、输入 #crontab -e (不推荐)

　　　 例如： 　　　

     * * * * * date >> /home/SystemDate     (每分钟查询日期，并记录在SystemDate文件中)                                            

　　b、输入#vim myCron.sh  创建.sh文件,编辑如下命令，并存盘退出

　　　　例如：　　

     date >> /home/SystemDate                                                                                                 

　　　　补充：

　　　　>>的意思是追加（适用场景：把一个文件的内容复制到另一个文件的末尾）

　　　　>  的意思是重定向，就要覆盖原来文件的内容

　　　　注意：要将myCron.sh 修改成执行文件　

     chmod 744 myCron.sh                                                                                                       

　　　　让myCron.sh 至少是  -rwxr--r-- 

 

           */2 * * * * /bin/sh /root/logs/myCron.sh 

　　　　1、下面就说，如何添加和编辑 Crontab　　

      #crontab -e                                                                                                              

          （包括默认编辑器，如果你的linux已经修改了VISUAL环境变数的话，那编辑器就你的算！）

　　　在操作crontab时，是对用户来说，所以请指定用户，当然不指定的话，肯定就是默认当前登陆用户
crontab -u username　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　   　

　　　2、列出用户当前的 Crontab　　　

    crontab -1　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　  　　　　

　　　　列出其他用户的Crontab　　

    #crontab -u username -l                                                                                          　　　　　　

　　　　3、删除用户当前的 Crontab　

    crontab -u username -r　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　

　　　　4、其他操作：

　　　　

            5、从实例中说明，也许你会看得很清楚：

　　　　 5.1 每一天的某个时刻执行（一天执行一次）,例如每天凌晨 2:00 执行

      0 2 * * * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　

　　　　 5.2 每一天的两个时刻执行（一天执行两次），例如 早上8：00 晚上20:00 都执行

       0 8,20 * * * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　

　　　　 5.3 每一分钟都执行一次

       * * * * * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　    　　　　　     

　　　　5.4 每个星期的固定的一天中的某个时刻，例如每个星期的星期一上午8：00执行

       0 8 * * 1 date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 　

　　　　5.5 每十分钟执行一次

       */10 * * * * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　    

　　　　5.6 让命令只在3月、4月、10执行　

       * * * 3,4,10 * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　    

　　　　5.7 每个星期中固定的某两天的某个固定时刻执行，例如 每个星期的星期一和星期三的上午8：00执行

       0 8 * * 1,3 date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 　　　

　　　　5.8 每四个小时执行一次

       0 */4 * * * date >>  /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　 　　　

　　　　5.9 每个星期中固定的某两天的某两个固定时刻执行，例如 每个星期的星期一和星期三的上午8:00和晚上20:00执行

       0 8,20 * * * date >> /home/SystemDate                                         　　　　　　　　　　　　　　　　　　　　　　　　　　

　　　　5.10 每三十秒执行一次

       * * * * * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　
       * * * * * sleep 30　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　       

　　　　5.11 每年的第一分钟 @yearly = 0 0 1 1 *

       0 0 1 1 * date >> /home/SystemDate   　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　

　　　　5.12 每个月的第一分钟 @monthly = 0 0 1 * * 

　　　　0 0 1 * * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　      

　　　　5.13 每个星期的第一分钟@weekly = 0 0 1 * * 

　　　　0 0 1 * * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　

　　　　5.14每天的第一分钟 @daily =0 0 * * * 

　　　　0 0 * * * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　

　　　　5.15每个小时的第一分钟 @hourly = 0 * * * *

　　　　0 * * * * date >> /home/SystemDate　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　

　　　　下面还有很多 Examples ,真的不能一一列了，都已经四点了；（第一次把排版弄这么好（相对于我来说哈）,做好一件事真的不容易）；



> 备注： 将如何不产生日志 ： >/dev/null 2>&1 &
查看 crontab日志：/var/log/cron
 

　　

 