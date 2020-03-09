FROM mediaserver_dep
WORKDIR /app/live/mediaserver
COPY . /app/live/mediaserver
EXPOSE 9981 3478 3478/udp
#CMD ./script/restart_all_service.sh && ./script/restart_test_service.sh && /bin/bash -c "while true;do sleep 1;done"
CMD ["/app/live/mediaserver/script/docker_init.sh"]