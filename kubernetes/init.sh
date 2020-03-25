sed -i 's/^KUBELET_EXTRA_ARGS=*/KUBELET_EXTRA_ARGS=--fail-swap-on=false/g' /etc/sysconfig/kubelet
kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/v0.12.0/Documentation/kube-flannel.yml

hostnamectl --static master.ip
kubeadm init --ignore-preflight-errors Swap --pod-network-cidr=10.244.0.0/16 --apiserver-advertise-address=192.168.88.133
kubectl taint nodes --all node-role.kubernetes.io/master-