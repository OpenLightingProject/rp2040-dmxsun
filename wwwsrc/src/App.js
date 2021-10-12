import React from "react";
import {
  HashRouter as Router,
  Switch,
  Route,
  Link
} from "react-router-dom";

import Home from "./Home.js";
import Console from "./Console.js";
import Log from "./Log.js";
import Wireless from "./Wireless.js";

import 'bootstrap/dist/css/bootstrap.min.css';

// TODO: I'm not yet sure if this is really required
//import 'bootstrap/dist/js/bootstrap.min.js';

export default function App() {
  // Have a global, optional URL "prefix" so one can have the App
  // served by localhost during development but talk to the API of
  // a REAL dongle
  window.urlPrefix = '';
  //window.urlPrefix = 'http://169.254.53.1'; // comment line if not used

  return (
    <Router>
      <div>
        <nav class="navbar navbar-expand-lg navbar-light bg-light">
          <span class="navbar-brand">ICON</span>
          <ul class="navbar-nav mr-auto">
            <li class="nav-item active">
              <Link to="/" class="nav-link">Home</Link>
            </li>
            <li class="nav-item active">
              <Link to="/console" class="nav-link">Console</Link>
            </li>
            <li class="nav-item">
              <Link to="/config" class="nav-link">Config</Link>
            </li>
            <li class="nav-item">
              <Link to="/wireless" class="nav-link">Wireless</Link>
            </li>
            <li class="nav-item">
              <Link to="/log" class="nav-link">Log</Link>
            </li>
          </ul>
        </nav>

        {/* A <Switch> looks through its children <Route>s and
            renders the first one that matches the current URL. */}
        <Switch>
          <Route path="/console">
            <Console />
          </Route>
          <Route path="/config">
            <Config />
          </Route>
          <Route path="/wireless">
            <Wireless />
          </Route>
          <Route path="/log">
            <Log />
          </Route>
          <Route path="/">
            <Home />
          </Route>
        </Switch>
      </div>
    </Router>
  );
}

function Config() {
  return <h2>Config</h2>;
}
