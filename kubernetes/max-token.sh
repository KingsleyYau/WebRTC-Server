# !/bin/bash

kubectl get secret -n kube-system -o jsonpath={.data.token} $(kubectl get secret -n kube-system | grep max | awk '{print $1}') | base64 -d && echo