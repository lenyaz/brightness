#!/bin/bash

sudo cp brightness-auto.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable brightness-auto.service
sudo systemctl start brightness-auto.service
sudo systemctl status brightness-auto.service