import React from 'react';
import * as Icon from 'react-bootstrap-icons';
import Slider from 'react-input-slider';

class Home extends React.Component {
    constructor() {
        super();
        this.state = {
            config: {},
            inFlight: false,
        };
        this.setBrightnessTimeout = undefined;
    }

    componentDidMount() {
        this.updateValues.bind(this)();
    }

    componentWillUnmount() {
    }

    updateValues() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.inFlight) {
            return;
        }

        this.setState({ inFlight: true });
        const url = window.urlPrefix + '/overview/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ inFlight: false }); this.updateValues(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Values fetched: ', result);
                        this.setState({ inFlight: false, config: result });
                    }
                }
            ).finally(
                () => { this.setState({ inFlight: false }); }
            );
    }

    setStatusLedBrightness(newValue) {
        // We do the writing behind a "timeout" so we only write
        // once every 100ms in order not to overload the device
        if (this.setBrightnessTimeout) {
            window.clearTimeout(this.setBrightnessTimeout);
        }
        this.setBrightnessTimeout = window.setTimeout(this.realSetStatusLedBrightness.bind(this), 100, newValue);
    }

    realSetStatusLedBrightness(newValue) {
        const url = window.urlPrefix + '/config/statusLeds/brightness/set.json?value=' + newValue;
        fetch(url).finally(() => { this.updateValues() })
    }

    render() {
        return (
            <div className="console" class="container-fluid">
                <div class="row">
                    <div class="col">
                        &nbsp;
                    </div>
                </div>

                <div class="row">
                    <div class="col">

                        <table class="table">
                            <tbody>
                                <tr>
                                    <td class="text-center">Board name:</td>
                                    <td>{this.state.config.boardName}</td>
                                </tr>
                                <tr>
                                    <td class="text-center">Config source:</td>
                                    <td>{this.state.config.configSource}</td>
                                </tr>
                                <tr>
                                    <td class="text-center">Dongle IP address:</td>
                                    <td>{this.state.config.ownIp}</td>
                                </tr>
                                <tr>
                                    <td class="text-center">IP address assigned to host:</td>
                                    <td>{this.state.config.hostIp}</td>
                                </tr>
                                <tr>
                                    <td class="text-center">Network mask:</td>
                                    <td>{this.state.config.ownMask}</td>
                                </tr>
                                <tr>
                                    <td class="text-center">Serial number:</td>
                                    <td>{this.state.config.serial}</td>
                                </tr>
                                <tr>
                                    <td class="text-center">Firmware version:</td>
                                    <td>{this.state.config.version}</td>
                                </tr>
                                <tr>
                                    <td class="text-center">Status LED brightness:</td>
                                    <td>
                                        {this.state.config.statusLedBrightness}
                                        &nbsp;&nbsp;
                                        <Slider
                                            axis="x"
                                            xstep={10}
                                            xmin={1}
                                            xmax={255}
                                            x={this.state.config.statusLedBrightness}
                                            onChange={({ x }) => this.setStatusLedBrightness(x)}
                                        />
                                    </td>
                                </tr>
                            </tbody>
                        </table>
                    </div>

                    <div class="col">{ this.state.inFlight && <div class="spinner-border spinner-border-sm" role="status"></div> }</div>
                </div>
            </div>
        );
    }
}
export default Home;