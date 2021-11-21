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
        fetch(window.urlPrefix + '/config/wireless/spectrum/get.json')
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
            <div className="accordion" id="accordionWireless">
                <div className="accordion-item">
                  <h2 className="accordion-header" id="headingSniffer">
                    <button className="accordion-button" type="button" data-bs-toggle="collapse" data-bs-target="#collapseSniffer" aria-expanded="true" aria-controls="collapseSniffer">
                      Mode "Sniffer": Wireless spectrum
                    </button>
                  </h2>
                  <div id="collapseSniffer" className="accordion-collapse collapse show" aria-labelledby="headingSniffer" data-bs-parent="#accordionWireless">
                    <div className="accordion-body container">
                      <div className="row">
                        <div className="col">
                          &nbsp;
                        </div>
                        <div className="col col-9">
                            <Bar ref={(reference) => this.chartReference = reference } data={this.state.spectrumData} options={options} />
                        </div>
                        <div className="col">
                          &nbsp;
                        </div>
                      </div>
                    </div>
                  </div>
                </div>
                <div className="accordion-item">
                  <h2 className="accordion-header" id="headingBroadcast">
                    <button className="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#collapseBroadcast" aria-expanded="false" aria-controls="collapseBroadcast">
                      Mode "Broadcast"
                    </button>
                  </h2>
                  <div id="collapseBroadcast" className="accordion-collapse collapse" aria-labelledby="headingBroadcast" data-bs-parent="#accordionWireless">
                    <div className="accordion-body">
                      <strong>This is the second item's accordion body.</strong> It is hidden by default, until the collapse plugin adds the appropriate classes that we use to style each element. These classes control the overall appearance, as well as the showing and hiding via CSS transitions. You can modify any of this with custom CSS or overriding our default variables. It's also worth noting that just about any HTML can go within the <code>.accordion-body</code>, though the transition does limit overflow.
                    </div>
                  </div>
                </div>
                <div className="accordion-item">
                  <h2 className="accordion-header" id="headingMesh">
                    <button className="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#collapseMesh" aria-expanded="false" aria-controls="collapseMesh">
                      Mode "Mesh"
                    </button>
                  </h2>
                  <div id="collapseMesh" className="accordion-collapse collapse" aria-labelledby="headingMesh" data-bs-parent="#accordionWireless">
                    <div className="accordion-body">
                      <strong>This is the third item's accordion body.</strong> It is hidden by default, until the collapse plugin adds the appropriate classes that we use to style each element. These classes control the overall appearance, as well as the showing and hiding via CSS transitions. You can modify any of this with custom CSS or overriding our default variables. It's also worth noting that just about any HTML can go within the <code>.accordion-body</code>, though the transition does limit overflow.
                    </div>
                  </div>
                </div>
            </div>
        );
    }
}
export default Wireless;