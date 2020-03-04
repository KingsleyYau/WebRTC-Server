FROM mediaserver_dep
COPY ./file /app/live/mediaserver
WORKDIR /app/live/mediaserver
EXPOSE 9877 9981 9082
CMD ./script/restart_all_service.sh && ./script/restart_test_service.sh && /bin/sh -c "while true; do sleep 1; done"