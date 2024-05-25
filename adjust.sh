ulimit -n 50000

sudo sysctl -w kern.maxfiles=200000
sudo sysctl -w kern.maxfilesperproc=100000
sudo sysctl -w net.inet.tcp.msl=7500
sudo sysctl -w net.inet.tcp.always_keepalive=1
sudo sysctl -w net.inet.tcp.keepidle=10000
sudo sysctl -w net.inet.tcp.keepintvl=10000
sudo sysctl -w net.inet.tcp.keepcnt=5
