import React from 'react';

import BoardStatus from './BoardStatus';
import StatusLeds from './StatusLeds';

class Home extends React.Component {
    constructor() {
        super();
        this.state = {
            config: {},
            ioBoards: {
                "base": {},
                "boards": [
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                ],
            },
            loading: false,
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
    }


    componentDidMount() {
        this.updateOverview.bind(this)();
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

    updateOverview() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.loading) {
            return;
        }

        this.setState({ loading: true });
        const url = window.urlPrefix + '/overview/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ loading: false }); this.updateOverview(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        this.setState({ loading: false, config: result });
                        this.updateIoBoards();
                    }
                }
            ).finally(
                () => {this.setState({ loading: false });}
            );
    }

    updateIoBoards() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.loading) {
            return;
        }

        this.setState({ loading: true });
        const url = window.urlPrefix + '/overview/ioBoards/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ loading: false }); this.updateIoBoards(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('IoBoards fetched: ', result);
                        this.setState({ loading: false, ioBoards: result });
                    }
                }
            ).finally(
                () => { this.setState({ loading: false }); }
            );
    }

    updateStatuLeds() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.loading) {
            return;
        }

        this.setState({ loading: true });
        const url = window.urlPrefix + '/overview/statusleds/get.json';
        fetch(url)
            .then(res => res.json())
            .then(
                (result) => {
                    if (result) {
                        console.log('StatusLeds fetched: ', result);
                        this.setState({ loading: false, leds: result });
                    }
                }
            ).finally(
                () => { this.setState({ loading: false }); }
            );
    }

    render() {
        return (
            <div className="container-fluid">
                <div className="row">
                    <div className="col">
                        &nbsp;
                    </div>
                </div>

                <div className="row">
                    <div className="col">
                        Status Leds (lower half shows blinking aspect):
                        <StatusLeds leds={this.state.leds} />
                    </div>

                    <div className="col">{ this.state.loading && <div className="spinner-border spinner-border-sm" role="status"></div> }</div>

                    <div className="col">
                        &nbsp;
                    </div>
                </div>

                <div className="row">
                    <div className="col">
                        &nbsp;
                    </div>
                </div>

                <div className="row">
                    <div className="col">
                        Running configuration and connected IO boards:
                        <BoardStatus withEdit={false} config={this.state.config} ioBoards={this.state.ioBoards} updateOverview={this.updateOverview.bind(this)} />
                    </div>
                </div>
            </div>
        );
    }
}
export default Home;
