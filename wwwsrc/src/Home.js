import React from 'react';
//import * as Icon from 'react-bootstrap-icons';
import Slider from 'react-input-slider';

class Home extends React.Component {
    constructor() {
        super();
        this.state = {
            config: {},
            inFlight: false,
            leds: [
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"}
            ],
            updateStatusLedsInterval: undefined,
        };
        this.setBrightnessTimeout = undefined;
    }


    componentDidMount() {
        this.updateValues.bind(this)();
        let interval = window.setInterval(this.updateStatuLeds.bind(this), 2000);
        this.setState({
            updateStatusLedsInterval: interval
        });
    }

    componentWillUnmount() {
        if (this.state.updateStatusLedsInterval) {
            window.clearInterval(this.state.updateStatusLedsInterval);
        }
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

    updateStatuLeds() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.inFlight) {
            return;
        }

        this.setState({ inFlight: true });
        const url = window.urlPrefix + '/overview/statusleds/get.json';
        fetch(url)
            .then(res => res.json())
            .then(
                (result) => {
                    if (result) {
                        console.log('StatusLeds fetched: ', result);
                        this.setState({ inFlight: false, leds: result });
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

                    <div class="col">
                        Status Leds (lower half shows blinking aspect):
                        <table class="table" style={{ padding: '0px' }}>
                            <tbody>
                                <tr style={{ padding: '0px' }}>
                                    <td style={{ textAlign: 'center' }}>
                                        <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[0].static + ' 0%, ' + this.state.leds[0].static + ' 59%, ' + this.state.leds[0].blink + ' 61%, ' + this.state.leds[0].blink + ' 100%)' }}></div><br />
                                        Slot 00
                                    </td>
                                    <td style={{ textAlign: 'center' }}>
                                        <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[1].static + ' 0%, ' + this.state.leds[1].static + ' 59%, ' + this.state.leds[1].blink + ' 61%, ' + this.state.leds[1].blink + ' 100%)' }}></div><br />
                                        Slot 01
                                    </td>
                                    <td style={{ textAlign: 'center' }}>
                                        <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[2].static + ' 0%, ' + this.state.leds[2].static + ' 59%, ' + this.state.leds[2].blink + ' 61%, ' + this.state.leds[2].blink + ' 100%)' }}></div><br />
                                        Slot 10
                                    </td>
                                    <td style={{ textAlign: 'center' }}>
                                        <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[3].static + ' 0%, ' + this.state.leds[3].static + ' 59%, ' + this.state.leds[3].blink + ' 61%, ' + this.state.leds[3].blink + ' 100%)' }}></div><br />
                                        Slot 11
                                    </td>
                                    <td style={{ textAlign: 'center' }}>
                                        <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[4].static + ' 0%, ' + this.state.leds[4].static + ' 59%, ' + this.state.leds[4].blink + ' 61%, ' + this.state.leds[4].blink + ' 100%)' }}></div><br />
                                        System
                                    </td>
                                    <td style={{ textAlign: 'center' }}>
                                        <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[5].static + ' 0%, ' + this.state.leds[5].static + ' 59%, ' + this.state.leds[5].blink + ' 61%, ' + this.state.leds[5].blink + ' 100%)' }}></div><br />
                                        USB
                                    </td>
                                    <td style={{ textAlign: 'center' }}>
                                        <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[6].static + ' 0%, ' + this.state.leds[6].static + ' 59%, ' + this.state.leds[6].blink + ' 61%, ' + this.state.leds[6].blink + ' 100%)' }}></div><br />
                                        Wireless
                                    </td>
                                    <td style={{ textAlign: 'center' }}>
                                        <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[7].static + ' 0%, ' + this.state.leds[7].static + ' 59%, ' + this.state.leds[7].blink + ' 61%, ' + this.state.leds[7].blink + ' 100%)' }}></div><br />
                                        Universes<br />in use
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