# AT HOST
# Creation of TAP
sudo ip tuntap add mode tap tap_hil
sudo ip link add test_bridge type bridge
sudo ip address add 192.168.56.66/24 dev test_bridge
sudo ip link set dev tap_hil address 00:00:00:00:00:66 up
sudo ip link set dev tap_hil master test_bridge
sudo ip link set dev vboxnet0 master test_bridge
sudo ip link set dev test_bridge up

# Check state
sudo sysctl net.bridge.bridge-nf-call-iptables
sudo sysctl net.bridge.bridge-nf-call-ip6tables
sudo sysctl net.bridge.bridge-nf-call-arptables
sudo sysctl net.ipv4.ip_forward

# Set state to 0 if needed
if [ $(sysctl net.bridge.bridge-nf-call-iptables) -eq 1 ]; then
    sudo sysctl net.bridge.bridge-nf-call-iptables=0
fi 
if [ $(sysctl net.bridge.bridge-nf-call-ip6tables) -eq 1 ]; then
    sudo sysctl net.bridge.bridge-nf-call-ip6tables=0
fi
if [ $(sysctl net.bridge.bridge-nf-call-arptables) -eq 1 ]; then
    sudo sysctl net.bridge.bridge-nf-call-arptables=0
fi

# In the VM
sudo arp -i enp0s8 -s 192.168.56.11 00:00:00:00:00:11