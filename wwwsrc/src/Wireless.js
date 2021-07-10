import React from 'react';

import couch64 from 'couch64';
import snappyjs from 'snappyjs';

import { Bar } from 'react-chartjs-2';

const options = {
    scales: {
        yAxes: [
            {
                ticks: {
                    beginAtZero: true,
                },
            },
        ],
    },
};

class Wireless extends React.Component {
    chartReference = {};

    constructor() {
        super();
        this.state = {
            updateSpectrumInterval: undefined,
            spectrumData: {
                labels: [],
                datasets: [
                    {
                        label: 'Usage of channel',
                        data: [],
                        fill: true,
                    },
                ],
            },
        };
        for (let i = 2400; i < 2517; i++) {
            this.state.spectrumData.labels.push(i);
        }
    }

    componentDidMount() {
        let interval = window.setInterval(this.updateSpectrum.bind(this), 2000);
        this.setState({
            updateSpectrumInterval: interval
        });
    }

    componentWillUnmount() {
        if (this.state.updateSpectrumInterval) {
            window.clearInterval(this.state.updateSpectrumInterval);
        }
    }

    updateSpectrum() {
        //fetch('http://169.254.230.1/config/wireless/spectrum/get.json')
        fetch("/config/wireless/spectrum/get.json")
            .then(res => res.json())
            .then(
                (result) => {
                    if (result) {
                        // TODO: Helper function to base64decode + snappy decompress
                        let compressed = couch64.base64DecToArr(result.spectrum);
                        let uncompressed = snappyjs.uncompress(compressed);
                        let newSpectrumData = this.state.spectrumData;
                        newSpectrumData.datasets[0].data.length = 0;
                        Array.prototype.push.apply(newSpectrumData.datasets[0].data, new Uint16Array(uncompressed.buffer));
                        this.chartReference.data = newSpectrumData;
                        this.chartReference.update();
                    }
                }
            );
    }
    render() {
        return (
            <div className="wireless">
                <Bar ref={(reference) => this.chartReference = reference } data={this.state.spectrumData} options={options} />
            </div>
        );
    }
}
export default Wireless;