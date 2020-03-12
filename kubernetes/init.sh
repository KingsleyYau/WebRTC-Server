sed -i 's/^KUBELET_EXTRA_ARGS=*/KUBELET_EXTRA_ARGS=--fail-swap-on=false/g' /etc/sysconfig/kubelet

kubeadm init --ignore-preflight-errors Swap --pod-network-cidr=10.244.0.0/16 --apiserver-advertise-address=192.168.88.133
kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/2140ac876ef134e0ed5af15c65e414cf26827915/Documentation/kube-flannel.yaml
kubectl taint nodes --all node-role.kubernetes.io/master-