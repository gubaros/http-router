ulimit -n 100000

sudo sysctl -w kern.maxfiles=204800
sudo sysctl -w kern.maxfilesperproc=204800
sudo sysctl -w kern.ipc.somaxconn=65535
sudo sysctl -w net.inet.tcp.msl=1000
sudo sysctl -w net.inet.tcp.rfc1323=1
sudo sysctl -w net.inet.tcp.always_keepalive=1
sudo sysctl -w net.inet.ip.forwarding=1
sudo sysctl -w net.inet.ip.ttl=65
sudo sysctl -w net.inet.tcp.delayed_ack=0
sudo sysctl -w net.inet.tcp.keepinit=5000
sudo sysctl -w net.inet.tcp.keepidle=5000
sudo sysctl -w net.inet.tcp.keepintvl=1000
sudo sysctl -w net.inet.tcp.mssdflt=1440
sudo sysctl -w net.inet.tcp.slowstart_flightsize=20
sudo sysctl -w net.inet.tcp.local_slowstart_flightsize=20

