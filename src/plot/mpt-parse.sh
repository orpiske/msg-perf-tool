#!/bin/bash

share_dir=`dirname $0`/../share/mpt
tmp_path=$3

if [[ -z $1 ]] ; then
	echo "You must inform either 'receiver' or 'sender'"

	exit 1
fi

if [[ -z $2 ]] ; then
	echo "You must inform the PID"

	exit 1
fi

echo "Creating the directory"
mkdir -p ${tmp_path}/$2



if [[ "$1" == "receiver" ]] ; then
        echo "Processing the receiver data"
	receiver_latency_report=${tmp_path}/$2/$1-latency-report.csv
	receiver_throughput_report=${tmp_path}/$2/$1-throughput-report.csv
        receiver_summary_report=${tmp_path}/$2/$1-summary-report.csv


        cat /dev/null > $receiver_latency_report
	cat /dev/null > $receiver_throughput_report
        total_count=0;
	for file in $LOG_DIR/mpt-$1-$2 ; do
		cat $LOG_DIR/mpt-$1-$2* | grep latency | sed 's/\[STAT\]: //' | sed 's/received:/received;/g' >> $receiver_latency_report
		cat $LOG_DIR/mpt-$1-$2*.log  | grep STAT | grep -v "summary" | grep rate | sed 's/\[STAT\]: //' >> $receiver_throughput_report
                summary_line=`cat $LOG_DIR/mpt-$1-$2* | grep summary`

                msg_count=`echo $summary_line | awk -F ';' ' { print $3 }'`
                
                let total_count=total_count+msg_count
        
	done

        echo "Processing the summary"
        test_date=`date`
        
        receiver_latency_report_in=$receiver_latency_report
	receiver_latency_report_out=${tmp_path}/$2/receiver-latency-report.png
	receiver_throughput_report_in=$receiver_throughput_report
	receiver_throughput_report_out=${tmp_path}/$2/receiver-throughput-report.png

	gnuplot -e "filename='$receiver_latency_report_in';output_filename='$receiver_latency_report_out'" ${share_dir}/latency.ps
	gnuplot -e "filename='$receiver_throughput_report_in';output_filename='$receiver_throughput_report_out'" ${share_dir}/throughput.ps

        env_data=`screenfetch -N`
        echo "{}" | jinja2 -D test_date="$test_date" -D environment="$env_data" -D total_count="$total_count" ${share_dir}/receiver-report.html > ${tmp_path}/$2/index.html
else
	if [[ $1 == "sender" ]] ; then
                echo "Processing the sender data"
		sender_throughput_report=${tmp_path}/$2/$1-throughput-report.csv

                total_count=0;
		cat /dev/null > $sender_throughput_report
		for file in $LOG_DIR/mpt-$1-$2 ; do
			cat $LOG_DIR/mpt-$1-$2*.log  | grep STAT | grep rate | sed 's/\[STAT\]: //' >> $sender_throughput_report

                        summary_line=`cat $LOG_DIR/mpt-$1-$2* | grep summary`

                        msg_count=`echo $summary_line | awk -F ';' ' { print $3 }'`
                
                        let total_count=total_count+msg_count
		done

                test_date=`date`
		sender_throughput_report_in=$sender_throughput_report
		sender_throughput_report_out=${tmp_path}/$2/sender-throughput-report.png

		gnuplot -e "filename='$sender_throughput_report_in';output_filename='$sender_throughput_report_out'" ${share_dir}/throughput.ps
                echo "{}" | jinja2 -D test_date="$test_date" -D environment="$env_data" -D total_count="$total_count" ${share_dir}/sender-report.html > ${tmp_path}/$2/index.html

	else
		echo "Invalid option $1"
	fi
fi